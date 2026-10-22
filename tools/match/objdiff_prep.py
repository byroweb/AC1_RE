#!/usr/bin/env python3
"""objdiff_prep.py - objdiff (github.com/encounter/objdiff) workstation for the match loop.

    objdiff_prep.py config [--unit U]... [--with-matched] [--prebuild] [-j N]
    objdiff_prep.py build  build/objdiff/base/<unit>/func_<ADDR>.o     (objdiff calls this)
    objdiff_prep.py one    <unit> <vaddr>                               (build both, print a summary)

config  writes objdiff.json at the repo root and one target object per function,
        build/objdiff/target/<unit>/func_<ADDR>.o, holding the retail bytes (the
        bytes match_fn.py compares against: --expect-size long when functions.tsv
        knows the size).  Units: every open function (functions.tsv, not PsyQ, not in
        the manifest), or only --unit U (repeatable); --with-matched adds the manifest
        rows too, marked complete.  --prebuild also builds every base object whose C
        file exists (-j N in parallel); otherwise objdiff builds the one you open.
build   compiles re/match/<unit>/func_<ADDR>.c (or the manifest row's source) into
        build/objdiff/base/<unit>/func_<ADDR>.o.  A missing C file gets drive.py's
        skeleton (m2c draft in a comment) first, so clicking any unit in objdiff
        gives you a file to edit.
one     builds target + base for one function and prints the word diff count, the
        same verdict match_fn.py would print.

The base object is made by match_fn.py's own functions with the argv drive.py try
(open functions) or check_all (manifest rows) would pass: same cpp/cc1/maspsx/as
flags, same symbol tables, auto --defsym for unresolved func_<ADDR>/D_<ADDR>,
--rodata from functions.tsv jt_addrs.  Both objects are flat: .text holds the final
bytes with relocations already applied (objdiff then compares exactly what the
oracle compares), and a word match_fn.py masks (unresolved symbol) gets the retail
value in its masked field, so it never shows as a diff - the oracle ignores it too.
The function symbol is func_<ADDR> of the retail address on both sides, whatever
the C names it (twin rows reuse another function's C).

objdiff only shows the diff.  The judge is still match_fn.py: once objdiff shows
no difference, run `python tools/loop/drive.py try <unit> <vaddr>` (it appends the
manifest row) and commit.
"""
import argparse
import concurrent.futures
import json
import os
import re
import shlex
import shutil
import struct
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
LOOP = os.path.join(REPO, "tools", "loop")
sys.path.insert(0, HERE)
sys.path.insert(0, LOOP)
import match_fn  # noqa: E402
import drive     # noqa: E402
import rank      # noqa: E402
import units     # noqa: E402

OUT = os.path.join(REPO, "build", "objdiff")
CONFIG = os.path.join(REPO, "objdiff.json")
AUTO_SYM = re.compile(r"^(func|D)_([0-9A-Fa-f]{8})$")
OBJ_PATH = re.compile(r"(?:^|[\\/])(target|base)[\\/]([A-Za-z0-9_]+)[\\/]func_([0-9A-Fa-f]{8})\.o$")
SCHEMA = "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json"
OPTIONS = {"mips.abi": "o32", "mips.instrCategory": "r3000gte"}
WATCH = ["*.c", "*.h", "*.inc", "ac1_markup.tsv", "include/ac1/symbols.tsv"]


class PrepError(Exception):
    pass


# ---------------------------------------------------------------------------
# minimal ELF32 little-endian MIPS relocatable: .text + one FUNC symbol
# ---------------------------------------------------------------------------
def elf_object(name, code):
    """Bytes of an ET_REL MIPS object whose .text is `code` and whose only symbol is the
    global function `name` covering all of it (e_flags 0 = mips1/o32, what `as -march=r3000`
    writes)."""
    shstr = b"\0.text\0.symtab\0.strtab\0.shstrtab\0"
    strtab = b"\0" + name.encode("ascii") + b"\0"
    symtab = bytes(16) + struct.pack("<IIIBBH", 1, 0, len(code), (1 << 4) | 2, 0, 1)
    body = bytearray(b"\0" * 52)
    offs = {}
    for key, blob, align in (("text", code, 4), ("symtab", symtab, 4), ("strtab", strtab, 1),
                             ("shstrtab", shstr, 1)):
        while len(body) % align:
            body += b"\0"
        offs[key] = len(body)
        body += blob
    while len(body) % 4:
        body += b"\0"
    shoff = len(body)
    sh = [struct.pack("<10I", *([0] * 10)),
          struct.pack("<10I", 1, 1, 6, 0, offs["text"], len(code), 0, 0, 4, 0),             # PROGBITS AX
          struct.pack("<10I", 7, 2, 0, 0, offs["symtab"], len(symtab), 3, 1, 4, 16),        # SYMTAB
          struct.pack("<10I", 15, 3, 0, 0, offs["strtab"], len(strtab), 0, 0, 1, 0),        # STRTAB
          struct.pack("<10I", 23, 3, 0, 0, offs["shstrtab"], len(shstr), 0, 0, 1, 0)]
    for s in sh:
        body += s
    ident = b"\x7fELF" + bytes([1, 1, 1, 0]) + bytes(8)
    struct.pack_into("<16sHHIIIIIHHHHHH", body, 0, ident, 1, 8, 1, 0, 0, shoff, 0,
                     52, 0, 0, 40, len(sh), 4)
    return bytes(body)


def write_object(path, name, code):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    tmp = path + ".tmp"
    with open(tmp, "wb") as fh:
        fh.write(elf_object(name, code))
    os.replace(tmp, path)


def unmask(ours, retail, masks):
    """Copy the retail bits under each match_fn mask into ours (oracle-equal words stay equal)."""
    out = bytearray(ours)
    for off, m in masks.items():
        if off + 4 > len(retail) or off + 4 > len(out):
            continue
        a = struct.unpack_from("<I", out, off)[0]
        b = struct.unpack_from("<I", retail, off)[0]
        struct.pack_into("<I", out, off, (a & ~m) | (b & m))
    return bytes(out)


def parse_obj_path(path):
    """build/objdiff/{target,base}/<unit>/func_<ADDR>.o -> (side, unit, 0xADDR)."""
    m = OBJ_PATH.search(path.replace("\\", "/"))
    if not m:
        raise PrepError("not an objdiff_prep object path: %s" % path)
    return m.group(1), units.norm(m.group(2)), "0x%s" % m.group(3).upper()


def obj_path(side, unit, vaddr):
    return os.path.join(OUT, side, unit, drive.fn_name(vaddr) + ".o")


def rel(path):
    return drive.rel(path)


# ---------------------------------------------------------------------------
# which functions, with which oracle argv
# ---------------------------------------------------------------------------
def manifest_rows():
    """{(unit, 0xADDR): manifest row} (fdat aliases and bad rows skipped)."""
    out = {}
    for r in drive.read_manifest()[1]:
        try:
            unit = units.norm(r["unit"] or "exe")
            out[(unit, drive.ledger.norm_vaddr(r["vaddr"]))] = r
        except (units.UnitError, ValueError):
            continue
    return out


def function_rows():
    return {(r["unit"], r["vaddr"]): r for r in rank.load_functions()}


def open_functions(rows, matched, only_units=None):
    out = []
    for r in rank.candidates(list(rows.values()), set(matched)):
        if only_units and r["unit"] not in only_units:
            continue
        out.append(r)
    out.sort(key=lambda r: (r["unit"], int(r["vaddr"], 16)))
    return out


def oracle_argv(unit, vaddr, rows, matched):
    """(match_fn argv without the toolchain-only switches, C path) for one function, the
    argv check_all (manifest row) or drive.py try (open function) would use."""
    u = units.resolve(unit)
    v = int(vaddr, 16)
    r = matched.get((unit, vaddr))
    if r:
        extra = shlex.split(r["extra"]) if r["extra"] else []
        if "--base" in u.oracle_flags and "--base" in extra:
            i = extra.index("--base")
            del extra[i:i + 2]
        cpath = os.path.join(drive.RE_MATCH, r["src"])
        argv = [vaddr, cpath] + list(u.oracle_flags)
        if r["size"]:
            argv += ["--expect-size", r["size"]]
        return argv + extra, cpath
    row = rows.get((unit, vaddr))
    size = int(row["size"]) if row else None
    rodata = None
    if row and int(row["jump_tables"]) and row.get("jt_addrs"):
        rodata = row["jt_addrs"].split(",")[0].strip()
    cpath = drive.c_path(drive.RE_MATCH, unit, vaddr)
    return drive.oracle_argv(u, v, cpath, size, rodata), cpath


def absolutize_syms(argv):
    """--syms paths are repo-relative (the oracle runs in the repo root)."""
    out = list(argv)
    for i, a in enumerate(out[:-1]):
        if a == "--syms" and not os.path.isabs(out[i + 1]):
            out[i + 1] = os.path.join(REPO, out[i + 1])
    return out


# ---------------------------------------------------------------------------
# the two objects
# ---------------------------------------------------------------------------
def retail_bytes(argv):
    args = match_fn.make_parser().parse_args(absolutize_syms(argv))
    path, base, _label = match_fn.pick_bin(args, args.vaddr)
    if not args.expect_size:
        raise PrepError("no retail size known for 0x%08X (not in functions.tsv)" % args.vaddr)
    with open(path, "rb") as fh:
        fh.seek(args.vaddr - base)
        data = fh.read(args.expect_size)
    if len(data) != args.expect_size:
        raise PrepError("0x%08X+%d runs past %s" % (args.vaddr, args.expect_size, path))
    return data


def compile_base(argv):
    """Run match_fn's pipeline on argv.  -> dict(ours, retail, masks, unresolved, fn)."""
    args = match_fn.make_parser().parse_args(absolutize_syms(argv))
    tc = match_fn.load_toolchain_json()
    tools = {"cc1": match_fn.find_cc1(args, tc), "as": match_fn.find_as(args, tc),
             "maspsx": match_fn.find_maspsx(args, tc), "cpp": match_fn.find_cpp(args, tc)}
    bin_path, bin_base, _label = match_fn.pick_bin(args, args.vaddr)
    work = tempfile.mkdtemp(prefix="objdiff_prep_")
    log = []
    try:
        obj, _asm = match_fn.compile_to_object(args, tools, args.src, work, log)
        with open(obj, "rb") as fh:
            elf = match_fn.Elf(fh.read())
    finally:
        shutil.rmtree(work, ignore_errors=True)
    fn, start, size = match_fn.pick_function(elf, args.fn)
    if size == 0 or size % 4:
        raise PrepError("function %s has size %d" % (fn, size))
    symtab = match_fn.load_symbols(args)
    for s in elf.symbols:                      # drive.py try's auto --defsym, in one pass
        m = AUTO_SYM.match(s["name"])
        if m and s["shndx"] == 0 and s["name"] not in symtab:
            symtab[s["name"]] = int(m.group(2), 16)
    ours, masks, _res, unresolved = match_fn.resolve_relocs(elf, start, size, args.vaddr - start,
                                                            symtab, args.rodata, args.data)
    with open(bin_path, "rb") as fh:
        fh.seek(args.vaddr - bin_base)
        retail = fh.read(max(size, args.expect_size or 0))
    return {"ours": unmask(ours, retail, masks), "retail": retail, "masks": masks,
            "unresolved": sorted(set(unresolved)), "fn": fn,
            "expect_size": args.expect_size}


def build_target(unit, vaddr, rows, matched):
    argv, _c = oracle_argv(unit, vaddr, rows, matched)
    path = obj_path("target", unit, vaddr)
    write_object(path, drive.fn_name(vaddr), retail_bytes(argv))
    return path


def ensure_source(unit, vaddr, cpath, rows):
    """drive.py next's skeleton (with an m2c draft when m2c is there) for a missing C file."""
    if os.path.isfile(cpath):
        return False
    row = dict(rows.get((unit, vaddr)) or {})
    if not row:
        raise PrepError("no C file %s and %s %s is not in functions.tsv" % (rel(cpath), unit, vaddr))
    row["score"] = rank.score(row)
    draft = None
    try:
        u = units.resolve(unit)
        gas, _err = drive.gas_text(u, int(vaddr, 16), int(row["size"]))
        draft, _err = drive.m2c_draft(gas)
    except SystemExit:
        draft = None
    drive.skeleton(cpath, row, draft)
    return True


def build_base(unit, vaddr, rows, matched, make_skeleton=True):
    argv, cpath = oracle_argv(unit, vaddr, rows, matched)
    if make_skeleton and ensure_source(unit, vaddr, cpath, rows):
        print("wrote skeleton %s" % rel(cpath), file=sys.stderr)
    res = compile_base(argv)
    path = obj_path("base", unit, vaddr)
    write_object(path, drive.fn_name(vaddr), res["ours"])
    res["path"] = path
    return res


def diff_words(ours, retail):
    """Count of differing words over the longer of the two (missing words count)."""
    n = max(len(ours), len(retail))
    d = 0
    for off in range(0, n, 4):
        a = ours[off:off + 4]
        b = retail[off:off + 4]
        if a != b:
            d += 1
    return d


# ---------------------------------------------------------------------------
# objdiff.json
# ---------------------------------------------------------------------------
def unit_entry(unit, vaddr, cpath, complete):
    name = "%s/%s" % (unit, drive.fn_name(vaddr))
    return {"name": name,
            "target_path": rel(obj_path("target", unit, vaddr)),
            "base_path": rel(obj_path("base", unit, vaddr)),
            "metadata": {"source_path": rel(cpath), "complete": complete,
                         "progress_categories": [unit]}}


def make_config(entries, cats):
    return {"$schema": SCHEMA,
            "min_version": "3.0.0",
            "custom_make": sys.executable.replace("\\", "/"),
            "custom_args": [rel(os.path.abspath(__file__)), "build"],
            "build_target": False,
            "build_base": True,
            "watch_patterns": WATCH,
            "ignore_patterns": ["build/**/*"],
            "options": OPTIONS,
            "progress_categories": [{"id": c, "name": c} for c in cats],
            "units": entries}


def cmd_config(args):
    rows = function_rows()
    matched = manifest_rows()
    only = {units.norm(u) for u in args.unit} if args.unit else None
    todo = [(r["unit"], r["vaddr"], False) for r in open_functions(rows, matched, only)]
    if args.with_matched:
        todo += [(u, v, True) for (u, v) in sorted(matched, key=lambda k: (k[0], int(k[1], 16)))
                 if not only or u in only]
    entries, skipped = [], []
    for unit, vaddr, done in todo:
        try:
            build_target(unit, vaddr, rows, matched)
            _argv, cpath = oracle_argv(unit, vaddr, rows, matched)
        except (PrepError, match_fn.ToolError, units.UnitError) as e:
            skipped.append("%s %s: %s" % (unit, vaddr, e))
            continue
        entries.append(unit_entry(unit, vaddr, cpath, done))
    cats = sorted({e["metadata"]["progress_categories"][0] for e in entries})
    with open(CONFIG, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(make_config(entries, cats), fh, indent=2)
        fh.write("\n")
    print("objdiff.json: %d units (%d open, %d matched), targets in %s"
          % (len(entries), sum(not e["metadata"]["complete"] for e in entries),
             sum(e["metadata"]["complete"] for e in entries), rel(os.path.join(OUT, "target"))))
    for s in skipped:
        print("skipped " + s)
    if args.prebuild:
        jobs = [(u, v) for (u, v, _d) in todo
                if os.path.isfile(oracle_argv(u, v, rows, matched)[1])]

        def one(k):
            try:
                build_base(k[0], k[1], rows, matched, make_skeleton=False)
                return None
            except (PrepError, match_fn.ToolError) as e:
                return "%s %s: %s" % (k[0], k[1], str(e).splitlines()[0])
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as ex:
            errs = [e for e in ex.map(one, jobs) if e]
        print("prebuilt %d base objects (%d failed to compile)" % (len(jobs) - len(errs), len(errs)))
        for e in errs:
            print("  " + e)
    return 0


def cmd_build(args):
    side, unit, vaddr = parse_obj_path(args.path)
    rows = function_rows()
    matched = manifest_rows()
    try:
        if side == "target":
            build_target(unit, vaddr, rows, matched)
        else:
            res = build_base(unit, vaddr, rows, matched)
            if res["unresolved"]:
                print("note: masked (unresolved) symbols: %s" % ", ".join(res["unresolved"]))
    except (PrepError, match_fn.ToolError, units.UnitError) as e:
        print("objdiff_prep: %s" % e, file=sys.stderr)
        return 2
    return 0


def cmd_one(args):
    unit = units.norm(args.unit)
    vaddr = drive.ledger.norm_vaddr(args.vaddr)
    rows = function_rows()
    matched = manifest_rows()
    try:
        t = build_target(unit, vaddr, rows, matched)
        res = build_base(unit, vaddr, rows, matched)
    except (PrepError, match_fn.ToolError, units.UnitError) as e:
        print("objdiff_prep: %s" % e, file=sys.stderr)
        return 2
    with open(t, "rb") as fh:
        retail = match_fn.Elf(fh.read()).section_bytes(".text")
    d = diff_words(res["ours"], retail)
    print("target %s (%d bytes)" % (rel(t), len(retail)))
    print("base   %s (%d bytes, from %s)" % (rel(res["path"]), len(res["ours"]), res["fn"]))
    if res["unresolved"]:
        print("masked: %s" % ", ".join(res["unresolved"]))
    print("%s %s %d differing word(s)" % ("SAME" if d == 0 else "DIFF", drive.fn_name(vaddr), d))
    return 0 if d == 0 else 1


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    c = sub.add_parser("config", help="write objdiff.json + target objects")
    c.add_argument("--unit", action="append", help="exe, ovl201..ovl204, fdatNNN (repeatable; default all)")
    c.add_argument("--with-matched", action="store_true", help="also list manifest rows (complete)")
    c.add_argument("--prebuild", action="store_true", help="also build base objects for existing C files")
    c.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4)
    b = sub.add_parser("build", help="build one object (objdiff's build command)")
    b.add_argument("path")
    o = sub.add_parser("one", help="build target + base for one function and compare")
    o.add_argument("unit")
    o.add_argument("vaddr")
    args = ap.parse_args(argv)
    os.chdir(REPO)
    return {"config": cmd_config, "build": cmd_build, "one": cmd_one}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
