#!/usr/bin/env python3
"""permute.py - unattended permuter tier: grind near-miss C drafts against the oracle.

    permute.py pick  (--report F ... | --cfile [UNIT:]C ...) [--max-words 2] [--max-ratio R]
                     [--max-size N] [--limit N] [--drafts DIR ...]
    permute.py run   (same picking options) [-j 24] [--budget 300] [--levers LIST]
                     [--lever-evals 150] [--beam 4] [--no-permuter]
                     [--work DIR] [--no-ledger] [--no-replay] [--dry-run]
    permute.py setup <unit> <vaddr> <draft.c> <dir>       (one permuter dir, for manual runs)
    permute.py cc    <dir> <in.c> -o <out.o>              (compile.sh backend, one candidate)

Two stages per function, both scored with exactly the oracle's compile
(tools/match/match_fn.py: cpp, cc1 2.7.2, rodata align fix, maspsx 2.56, as, then the
oracle's relocation resolution: --syms tables, func_<ADDR>/D_<ADDR> names, --rodata):

  1. levers (tools/loop/levers.py): deterministic rewrites of the draft for the
     GCC 2.7.2 / m2c divergences in the match-function skill table (indexed global
     -> named array, pointee type/signedness of pointer locals and externs, the
     other-signedness spelling of a literal, adjacent statement swap, commuted
     operands), beam search (--beam best texts per depth, --lever-evals compiles in
     all) scored by differing words + 2 x size difference in words;
  2. simonlindholm/decomp-permuter ($PERMUTER_DIR, default ~/dev/decomp-permuter; its
     .venv python when present) on the best lever text for the rest of --budget
     seconds, until score 0 (--stop-on-zero --stack-diffs).  Its -j N needs fork() and
     is broken on Windows (its own comment in src/main.py), so each function gets one
     single-threaded permuter process and `run` keeps -j functions going at once.
     tools/loop/permuter_main.py runs it with the compile step in-process (no bash, no
     python or maspsx start-up per candidate).

A permuter directory (`setup`) holds base.c (the text run through the oracle's cpp),
target.o (the retail words under the function's symbol), compile.sh (`permute.py cc`,
the same compile for a stock permuter run), settings.toml and meta.json.

A score-0 text (lever text, or the permuter's function spliced back into the lever
text) goes through match_fn.py like `drive.py try` (unresolved func_/D_ names become
--defsym, rerun once), and only a clean MATCH (exit 0, no masked word, size equal) is
written to re/match/<unit>/func_<ADDR>.c, verified again from there, and given its
manifest row and ledger row (match, attempts 0, reason `permuter (...)`).  Nothing
is recorded for a miss.  <work>/permute_report.tsv has one row per function.

`pick` ranks candidates from sweep reports (sweep.py's sweep_report.tsv: the best
MISMATCH try of each body that is still open and has no re/match C file) and from
--cfile near-miss / give-up C files (`<unit>/func_<ADDR>.c`, a sweep work dir's
`<unit>_<ADDR>_<variant>.c`, or any file as UNIT:PATH with func_<ADDR> in its name) by
differing words relative to size, words / (size/4).
"""
import argparse
import datetime
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
MATCH_DIR = os.path.join(REPO, "tools", "match")
sys.path.insert(0, HERE)
sys.path.insert(0, MATCH_DIR)
import match_fn  # noqa: E402  (stdlib only: keeps `cc` start-up cheap)

SYMS_TABLES = ["include/ac1/symbols.tsv", "ac1_markup.tsv"]
AUTO_SYM = re.compile(r"^(func|D)_([0-9A-Fa-f]{8})$")
TRY = re.compile(r"^(\S+) MISMATCH@0x([0-9a-f]+)/(\d+)(?:~[0-9.]+)?$")   # sweep adds ~closeness
SWEEP_C = re.compile(r"^(exe|ovl20[1-4]|fdat\d{3})_([0-9A-Fa-f]{8})_([\w-]+)\.c$")
FUNC_C = re.compile(r"func_([0-9A-Fa-f]{8})\b")
SHT_NULL = 0


def permuter_dir():
    d = os.environ.get("PERMUTER_DIR") or os.path.join(os.path.expanduser("~"), "dev", "decomp-permuter")
    if not os.path.isfile(os.path.join(d, "permuter.py")):
        raise SystemExit("permute.py: no decomp-permuter at %s (git clone "
                         "https://github.com/simonlindholm/decomp-permuter there, or set PERMUTER_DIR)" % d)
    return d


def permuter_python(d):
    for p in (os.path.join(d, ".venv", "Scripts", "python.exe"), os.path.join(d, ".venv", "bin", "python")):
        if os.path.isfile(p):
            return p
    return sys.executable


def toolchain():
    """The oracle's tools, resolved the way match_fn.main does."""
    ns = argparse.Namespace(cc1=None, as_=None, maspsx=None, cpp=None)
    tc = match_fn.load_toolchain_json()
    t = {"cc1": match_fn.find_cc1(ns, tc), "as": match_fn.find_as(ns, tc),
         "maspsx": match_fn.find_maspsx(ns, tc), "cpp": match_fn.find_cpp(ns, tc)}
    t["objdump"] = match_fn.find_objdump(t["as"])
    return t


_TOOLS = {}
_TOOLS_LOCK = threading.Lock()


def tools_once():
    with _TOOLS_LOCK:
        if not _TOOLS:
            _TOOLS.update(toolchain())
    return dict(_TOOLS)


def run(cmd, cwd=None):
    p = subprocess.run(cmd, cwd=cwd, stdin=subprocess.DEVNULL, capture_output=True, text=True)
    if p.returncode != 0:
        raise RuntimeError("%s failed (exit %d): %s%s" % (os.path.basename(cmd[0]), p.returncode,
                                                         p.stdout[-400:], p.stderr[-400:]))
    return p.stdout


# ---------------------------------------------------------------------------
# compile: the oracle's pipeline, reloc-free output
# ---------------------------------------------------------------------------
def load_meta(d):
    with open(os.path.join(d, "meta.json"), encoding="utf-8") as fh:
        return json.load(fh)


_MASPSX = {}
_MASPSX_LOCK = threading.Lock()


def maspsx_inproc(script, asm):
    """maspsx.py's main() for `--aspsx-version=2.56 fn.fixed.s` without a python start-up
    (~0.25 s on this PC): same processor, same config, same text (line ends aside).
    tools/match/tests/test_permute.py checks it against the subprocess."""
    with _MASPSX_LOCK:
        if script not in _MASPSX:
            import runpy
            sys.path.insert(0, os.path.dirname(os.path.abspath(script)))
            _MASPSX[script] = runpy.run_path(script, run_name="maspsx_script")
    g = _MASPSX[script]
    cfg = g["config_for_aspsx_version"](match_fn.ASPSX_VERSION)
    proc = g["MaspsxProcessor"](
        asm.splitlines(keepends=True), sdata_limit=0, expand_div=False, expand_li=cfg.expand_li,
        nop_at_expansion=cfg.nop_at_expansion, nop_mflo_mfhi=cfg.nop_mflo_mfhi, nop_lw_lw=cfg.nop_lw_lw,
        sltu_at=cfg.sltu_at, addiu_at=cfg.addiu_at, div_uses_tge=cfg.div_uses_tge,
        gp_allow_offset=cfg.gp_allow_offset, gp_allow_la=cfg.gp_allow_la, gp_allow=cfg.gp_allow,
        use_comm_section=False, use_comm_for_lcomm=False, max_comm_alignment=16,
        lcomm_alignment=cfg.lcomm_alignment, comm_uses_sdata_limit=cfg.comm_uses_sdata_limit)
    return "\n".join([""] + proc.process_lines()) + "\n"


def build_object(meta, src, work, tools):
    """cc1 -> align fix -> maspsx -> as on an already preprocessed source; -> .o path."""
    if os.path.abspath(src) != os.path.abspath(os.path.join(work, "fn.i")):
        shutil.copyfile(src, os.path.join(work, "fn.i"))
    run(match_fn.cc1_command(tools["cc1"]) + ["fn.i"] + match_fn.CC1_FLAGS + ["-o", "fn.s"], work)
    with open(os.path.join(work, "fn.s"), encoding="utf-8", errors="replace") as fh:
        asm = fh.read()
    if not meta.get("no_rodata_align_fix"):
        asm, _n = match_fn.fix_rodata_align(asm)
    with open(os.path.join(work, "fn.maspsx.s"), "w", newline="\n") as fh:
        fh.write(maspsx_inproc(tools["maspsx"], asm))
    run([tools["as"]] + match_fn.AS_FLAGS + ["-o", "fn.o", "fn.maspsx.s"], work)
    return os.path.join(work, "fn.o")


class AutoSyms(dict):
    """The oracle's symbol table plus what `drive.py try` adds: an unresolved
    func_<ADDR> / D_<ADDR> is its own address."""
    def get(self, k, default=None):
        if k in self:
            return self[k]
        m = AUTO_SYM.match(k or "")
        return int(m.group(2), 16) if m else default


def symtab(meta):
    ns = argparse.Namespace(syms=[os.path.join(REPO, f) for f in meta["syms"]],
                            defsym=list(meta["defsyms"]), no_markup=True)
    return AutoSyms(match_fn.load_symbols(ns))


def resolve_object(obj_path, out_path, meta, syms):
    """Apply the oracle's relocation resolution to the function, write .text back, drop
    .rel.text (out_path None: write nothing).  -> (function bytes, still-unresolved names)."""
    with open(obj_path, "rb") as fh:
        data = bytearray(fh.read())
    elf = match_fn.Elf(bytes(data))
    _name, start, size = match_fn.pick_function(elf, meta["fn"])
    text_vaddr = int(meta["vaddr"], 16) - start
    rodata = int(meta["rodata"], 16) if meta.get("rodata") else None
    fn_bytes, _masks, _res, unresolved = match_fn.resolve_relocs(elf, start, size, text_vaddr, syms,
                                                                rodata, None)
    if out_path:
        text = elf.by_name[".text"]
        data[text["offset"] + start:text["offset"] + start + size] = fn_bytes
        rel = elf.by_name.get(".rel.text")
        if rel:
            struct.pack_into("<I", data, elf.shoff + rel["idx"] * elf.shentsize + 4, SHT_NULL)
        with open(out_path, "wb") as fh:
            fh.write(data)
    return fn_bytes, sorted(set(unresolved))


def compile_candidate(meta, syms, source, out_path, show_errors=False):
    """Preprocessed C text -> reloc-free out_path.  True on success."""
    work = tempfile.mkdtemp(prefix="perm_cc_")
    try:
        src = os.path.join(work, "fn.i")
        with open(src, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(source)
        resolve_object(build_object(meta, src, work, meta["tools"]), out_path, meta, syms)
        return True
    except (RuntimeError, match_fn.ToolError, OSError, ValueError) as e:
        if show_errors:
            sys.stderr.write(str(e) + "\n")
        return False
    finally:
        shutil.rmtree(work, ignore_errors=True)


def cmd_cc(args):
    meta = load_meta(args.dir)
    with open(args.src, encoding="utf-8", errors="replace") as fh:
        source = fh.read()
    return 0 if compile_candidate(meta, symtab(meta), source, args.out, show_errors=True) else 1


def preprocess(tools, cpath, out_path, extra_inc=()):
    incs = []
    for d in [os.path.dirname(os.path.abspath(cpath))] + list(extra_inc) + [os.path.join(MATCH_DIR, "include")]:
        incs += ["-I", d]
    cpp = tools["cpp"]
    run([cpp] + (["-x", "c"] if "gcc" in os.path.basename(cpp) else []) + match_fn.CPP_FLAGS +
        incs + [os.path.abspath(cpath), "-o", out_path])


def text_bytes(meta, syms, text, inc_dir):
    """Draft-form C text -> (function bytes, unresolved) or None on a build error."""
    work = tempfile.mkdtemp(prefix="perm_lv_")
    try:
        c = os.path.join(work, "fn.c")
        with open(c, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        preprocess(meta["tools"], c, os.path.join(work, "fn.i"), [inc_dir])
        return resolve_object(build_object(meta, os.path.join(work, "fn.i"), work, meta["tools"]),
                              None, meta, syms)
    except (RuntimeError, match_fn.ToolError, OSError, ValueError):
        return None
    finally:
        shutil.rmtree(work, ignore_errors=True)


def word_score(ours, retail):
    """Differing words over the common length + 2 per word of size difference."""
    n = min(len(ours), len(retail)) // 4
    a = struct.unpack_from("<%dI" % n, ours)
    b = struct.unpack_from("<%dI" % n, retail)
    return sum(1 for x, y in zip(a, b) if x != y) + 2 * abs(len(ours) - len(retail)) // 4


# ---------------------------------------------------------------------------
# setup: one permuter directory per function
# ---------------------------------------------------------------------------
def retail_bytes(unit, vaddr, size):
    import units
    u = units.resolve(unit)
    with open(u.image_path, "rb") as fh:
        fh.seek(vaddr - u.base)
        b = fh.read(size)
    if len(b) != size:
        raise RuntimeError("short read of %s at 0x%08X" % (u.image_path, vaddr))
    return b


def write_target(tools, fn, blob, d):
    lines = [".set noreorder", ".set noat", ".text", ".globl %s" % fn, ".type %s, @function" % fn,
             "%s:" % fn]
    for i in range(0, len(blob), 4):
        lines.append(".word 0x%08x" % struct.unpack_from("<I", blob, i)[0])
    lines.append(".size %s, %d" % (fn, len(blob)))
    s = os.path.join(d, "target.s")
    with open(s, "w", newline="\n") as fh:
        fh.write("\n".join(lines) + "\n")
    run([tools["as"]] + match_fn.AS_FLAGS + ["-o", os.path.join(d, "target.o"), s])


def posix(p):
    p = os.path.abspath(p).replace("\\", "/")
    m = re.match(r"^([A-Za-z]):/(.*)$", p)
    return "/%s/%s" % (m.group(1).lower(), m.group(2)) if m else p


def setup_dir(unit, vaddr, cpath, d, size=None, rodata=None, tools=None):
    """Make permuter dir d for (unit, vaddr) from draft cpath.  -> meta dict."""
    import drive
    tools = tools or tools_once()
    vs = "0x%08X" % vaddr
    row = drive.load_rows().get((unit, vs))
    if size is None:
        if not row:
            raise RuntimeError("%s %s not in functions.tsv; pass the size" % (unit, vs))
        size = int(row["size"])
    if rodata is None and row and int(row.get("jump_tables") or 0) and row.get("jt_addrs"):
        rodata = row["jt_addrs"].split(",")[0].strip()
    os.makedirs(d, exist_ok=True)
    fn = drive.fn_name(vs)
    shutil.copyfile(cpath, os.path.join(d, "draft.c"))
    preprocess(tools, cpath, os.path.join(d, "base.c"))
    write_target(tools, fn, retail_bytes(unit, vaddr, size), d)
    meta = {"unit": unit, "vaddr": vs, "fn": fn, "size": size, "rodata": rodata,
            "syms": SYMS_TABLES if drive.uses_ac1_h(cpath) else [], "defsyms": [],
            "draft": os.path.abspath(cpath), "tools": tools}
    write_meta(d, meta)
    py = posix(sys.executable)
    with open(os.path.join(d, "compile.sh"), "w", newline="\n") as fh:
        fh.write('#!/usr/bin/env bash\n# permute.py cc: match_fn.py pipeline + oracle reloc resolution\n'
                 'exec "%s" "%s" cc "%s" "$1" -o "$3"\n' % (py, posix(__file__), posix(d)))
    objdump = (tools["objdump"] or "mipsel-none-elf-objdump").replace("\\", "/")
    with open(os.path.join(d, "settings.toml"), "w", newline="\n") as fh:
        fh.write('func_name = "%s"\ncompiler_type = "gcc"\nobjdump_command = "%s -drz -m mips:3000"\n'
                 % (fn, objdump))
    return meta


def write_meta(d, meta):
    with open(os.path.join(d, "meta.json"), "w", encoding="utf-8", newline="\n") as fh:
        json.dump(meta, fh, indent=1)


def cmd_setup(args):
    meta = setup_dir(args.unit, int(args.vaddr, 16), args.cfile, args.dir)
    print(json.dumps({k: meta[k] for k in ("unit", "vaddr", "size", "rodata", "syms")}))
    return 0


# ---------------------------------------------------------------------------
# stage 1: lever beam search
# ---------------------------------------------------------------------------
def lever_search(meta, syms, draft_text, retail, inc_dir, names, max_evals, beam, log=None):
    """-> dict(base, best, text, trail, evals).  Children whose compiled bytes were seen
    before (no-op rewrites) are dropped; equal scores are kept (plateau moves)."""
    import levers
    first = text_bytes(meta, syms, draft_text, inc_dir)
    res = {"base": None, "best": None, "text": draft_text, "trail": [], "evals": 1}
    if first is None:
        return res
    res["base"] = res["best"] = word_score(first[0], retail)
    seen_text = {hashlib.sha1(draft_text.encode()).digest()}
    seen_code = {first[0]}
    frontier = [(res["best"], draft_text, [])]
    while frontier and res["evals"] < max_evals and res["best"] > 0:
        scored = []
        for _sc, text, trail in frontier:
            for label, nt in levers.variants(text, meta["fn"], names):
                h = hashlib.sha1(nt.encode()).digest()
                if h in seen_text:
                    continue
                seen_text.add(h)
                if res["evals"] >= max_evals:
                    break
                out = text_bytes(meta, syms, nt, inc_dir)
                res["evals"] += 1
                if out is None or out[0] in seen_code:
                    continue
                seen_code.add(out[0])
                s = word_score(out[0], retail)
                if log:
                    log.write("%4d  %3d  %s\n" % (res["evals"], s, " + ".join(trail + [label])))
                scored.append((s, nt, trail + [label]))
                if s < res["best"] or (s == res["best"] and len(trail) + 1 < len(res["trail"])):
                    res.update(best=s, text=nt, trail=trail + [label])
                if s == 0:
                    return res
        scored.sort(key=lambda x: (x[0], len(x[2])))
        frontier = [x for x in scored if x[0] <= res["best"] + 1][:beam]
    return res


# ---------------------------------------------------------------------------
# pick: candidates from sweep reports and near-miss C files
# ---------------------------------------------------------------------------
def report_candidates(paths):
    """{(unit, vaddr): cand} from sweep_report.tsv files: best MISMATCH try per body."""
    out = {}
    for p in paths:
        with open(p, encoding="utf-8") as fh:
            header = fh.readline().rstrip("\r\n").split("\t")
            for line in fh:
                r = dict(zip(header, line.rstrip("\r\n").split("\t")))
                if r.get("status") == "MATCH":
                    continue
                for t in r.get("tries", "").split(" | "):
                    m = TRY.match(t.strip())
                    if not m:
                        continue
                    c = {"unit": r["unit"], "vaddr": "0x%08X" % int(r["vaddr"], 16),
                         "size": int(r["size"]), "variant": m.group(1),
                         "first": int(m.group(2), 16), "words": int(m.group(3)),
                         "n_members": int(r.get("n_members") or 1), "src": "report:" + p, "cfile": None}
                    k = (c["unit"], c["vaddr"])
                    if k not in out or c["words"] < out[k]["words"]:
                        out[k] = c
    return out


def cfile_candidate(spec):
    """`[UNIT:]PATH` -> candidate.  The unit comes from UNIT, a sweep draft name
    (<unit>_<ADDR>_<variant>.c) or the parent directory (re/match/<unit>/func_<ADDR>.c)."""
    unit = None
    m = re.match(r"^(exe|ovl20[1-4]|fdat\d{3}):(.+)$", spec)
    if m:
        unit, spec = m.group(1), m.group(2)
    msys = re.match(r"^/([A-Za-z])/(.*)$", spec)     # Git Bash leaves UNIT:/c/... unconverted
    if msys and os.name == "nt":
        spec = "%s:/%s" % (msys.group(1).upper(), msys.group(2))
    path = os.path.abspath(spec)
    base = os.path.basename(path)
    sm = SWEEP_C.match(base)
    if sm:
        unit, addr, variant = unit or sm.group(1), sm.group(2), sm.group(3)
    else:
        fm = FUNC_C.search(base)
        if not fm:
            raise SystemExit("cannot tell the address from %s (want func_<ADDR> in the name)" % spec)
        addr, variant = fm.group(1), "file"
        unit = unit or os.path.basename(os.path.dirname(path))
    if not re.match(r"^(exe|ovl20[1-4]|fdat\d{3})$", unit):
        raise SystemExit("cannot tell the unit of %s: pass it as UNIT:PATH" % spec)
    return {"unit": unit, "vaddr": "0x%08X" % int(addr, 16), "size": None, "variant": variant,
            "first": None, "words": None, "n_members": 1, "src": "cfile", "cfile": path}


def pick(args):
    import drive
    import ledger
    cands = report_candidates(args.report or [])
    for p in args.cfile or []:
        c = cfile_candidate(p)
        cands[(c["unit"], c["vaddr"])] = c
    rows = drive.load_rows()
    current = ledger.load()
    out = []
    for k, c in cands.items():
        row = rows.get(k)
        if c["size"] is None and row:
            c["size"] = int(row["size"])
        state = ledger.state(c["unit"], c["vaddr"], current)
        if state == "match" or (state == "giveup" and c["src"] != "cfile"):
            continue          # a give-up's own C file (--cfile) is exactly what this tier is for
        if c["src"] != "cfile" and os.path.exists(drive.c_path(drive.RE_MATCH, c["unit"], c["vaddr"])):
            continue
        if c["size"] and args.max_size and c["size"] >= args.max_size:
            continue
        if c["words"] is not None:
            if args.max_words is not None and c["words"] > args.max_words:
                continue
            c["ratio"] = c["words"] / max(1.0, c["size"] / 4.0)
            if args.max_ratio is not None and c["ratio"] > args.max_ratio:
                continue
        else:
            c["ratio"] = 0.0
        out.append(c)
    out.sort(key=lambda c: (c["ratio"], c["size"] or 0, c["unit"], c["vaddr"]))
    if args.limit:
        out = out[:args.limit]
    return out


def cmd_pick(args):
    for c in pick(args):
        print("%-7s %s %5s B  %-12s words %-3s ratio %.3f  %s" % (
            c["unit"], c["vaddr"], c["size"], c["variant"],
            "-" if c["words"] is None else c["words"], c["ratio"], c["cfile"] or c["src"]))
    return 0


def draft_file(c, drafts_dirs, workdir):
    """Path of the draft C for a candidate: --cfile, else <unit>_<ADDR>_<variant>.c in a
    --drafts dir or next to the report (sweep.py --work keeps them there), else
    regenerated with sweep.draft()."""
    if c["cfile"]:
        return c["cfile"]
    name = "%s_%s_%s.c" % (c["unit"], c["vaddr"][2:], c["variant"])
    dirs = list(drafts_dirs or [])
    if c["src"].startswith("report:"):
        dirs.append(os.path.dirname(os.path.abspath(c["src"][7:])))
    for dd in dirs:
        if os.path.isfile(os.path.join(dd, name)):
            return os.path.join(dd, name)
    import drive
    import sweep
    row = drive.load_rows()[(c["unit"], c["vaddr"])]
    p = os.path.join(workdir, "drafts", name)
    os.makedirs(os.path.dirname(p), exist_ok=True)
    with open(p, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(sweep.draft(row, c["variant"]))
    return p


# ---------------------------------------------------------------------------
# stage 2: the permuter; splice; oracle; record
# ---------------------------------------------------------------------------
def find_function(text, fn):
    """(start, end) of the definition of fn (start of its line .. closing brace) or None.
    A prototype (`T fn(...);`) is skipped: only `fn(...)` followed by `{` counts."""
    for m in re.finditer(r"^[^\n;{}#]*\b%s\s*\(" % re.escape(fn), text, re.M):
        depth, j = 1, m.end()
        while j < len(text) and depth:
            depth += {"(": 1, ")": -1}.get(text[j], 0)
            j += 1
        k = j
        while k < len(text) and text[k].isspace():
            k += 1
        if k >= len(text) or text[k] != "{":
            continue
        depth = 0
        for j in range(k, len(text)):
            if text[j] == "{":
                depth += 1
            elif text[j] == "}":
                depth -= 1
                if depth == 0:
                    return m.start(), j + 1
        return None
    return None


def splice(draft_text, perm_text, fn):
    """Replace fn's definition in the draft with the permuter's; None if either is missing."""
    a = find_function(draft_text, fn)
    b = find_function(perm_text, fn)
    if not a or not b:
        return None
    return draft_text[:a[0]] + perm_text[b[0]:b[1]] + draft_text[a[1]:]


STATUS = re.compile(r"iteration (\d+), (\d+) errors, score = (\d+)")
BASE = re.compile(r"base score = (\d+)")


def run_permuter(d, budget, log_path):
    """One single-threaded permuter on dir d until score 0 or budget s.
    -> dict(base, best, iterations, seconds, zero_dir, failed)."""
    pd = permuter_dir()
    cmd = [permuter_python(pd), os.path.join(HERE, "permuter_main.py"), pd, d, "--stop-on-zero",
           "--stack-diffs", "--best-only", "-j", "1"]
    env = dict(os.environ)
    cpp_dir = os.path.dirname(tools_once()["cpp"])
    bash_dir = os.path.dirname(shutil.which("bash") or "")
    env["PATH"] = os.pathsep.join([p for p in (cpp_dir, bash_dir) if p] + [env.get("PATH", "")])
    env["PYTHONIOENCODING"] = "utf-8"
    t0 = time.time()
    out = {"base": None, "best": None, "iterations": 0, "errors": 0, "zero_dir": None, "failed": None}
    tail = []
    with open(log_path, "w", encoding="utf-8", newline="\n") as log:
        p = subprocess.Popen(cmd, cwd=d, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, env=env)
        killer = threading.Timer(budget, p.kill)
        killer.start()
        buf = b""
        try:
            while True:
                chunk = p.stdout.read1(4096)
                if not chunk:
                    break
                buf += chunk
                parts = re.split(rb"[\r\n]", buf)
                buf = parts.pop()
                for raw in parts:
                    line = raw.decode("utf-8", "replace")
                    if not line.strip():
                        continue
                    m = STATUS.search(line)
                    if m:
                        out["iterations"], out["errors"] = int(m.group(1)), int(m.group(2))
                        continue
                    log.write(line + "\n")
                    tail = (tail + [line])[-3:]
                    m = BASE.search(line)
                    if m:
                        out["base"] = int(m.group(1))
        finally:
            p.wait()
            timed_out = not killer.is_alive() and p.returncode != 0
            killer.cancel()
    out["seconds"] = time.time() - t0
    if out["base"] is None and not timed_out:
        out["failed"] = "permuter exit %s: %s" % (p.returncode, " / ".join(tail)[-160:])
    names = os.listdir(d)
    zeros = sorted(x for x in names if x.startswith("output-0-"))
    scores = [int(x.split("-")[1]) for x in names if re.match(r"output-\d+-\d+$", x)]
    if zeros:
        out["zero_dir"] = os.path.join(d, zeros[0])
    if out["base"] is not None:
        out["best"] = min(scores + [out["base"]])
    return out


def oracle(c, cpath, meta):
    """match_fn.py on cpath like drive.py try (auto --defsym rerun).  -> (clean, defsyms, summary)."""
    import sweep
    import units
    out = sweep.oracle(units.resolve(c["unit"]), int(c["vaddr"], 16), cpath, meta["size"], meta["rodata"])
    return tuple(out[:3])          # sweep.oracle may return more (facts, ...) after these three


def commit(c, meta, text, defsyms, how, args):
    """Write text to re/match, oracle it from there, then manifest + ledger. -> status."""
    import drive
    import ledger
    import units
    unit, vs = c["unit"], c["vaddr"]
    cpath = drive.c_path(drive.RE_MATCH, unit, vs)
    old = None
    if os.path.exists(cpath):
        if c["src"] != "cfile" or os.path.abspath(c["cfile"]) != os.path.abspath(cpath):
            return "skipped: %s appeared meanwhile" % drive.rel(cpath)
        with open(cpath, encoding="utf-8") as fh:
            old = fh.read()
        text = re.sub(r"^/\* NOT MATCHED:[^\n]*\*/\n", "", text)
    header = ("/* %s %s  size %s  permuter tier (tools/loop/permute.py): %s, from the %s draft, %s */\n"
              % (unit, vs, meta["size"], how, c["variant"], datetime.date.today().isoformat()))
    os.makedirs(os.path.dirname(cpath), exist_ok=True)
    with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(header + text)
    clean, defsyms2, summ = oracle(c, cpath, meta)
    if not clean:
        if old is None:
            os.unlink(cpath)
        else:
            with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(old)
        return "re-verify failed (%s)" % summ
    u = units.resolve(unit)
    extra = []
    if meta["rodata"]:
        extra += ["--rodata", meta["rodata"]]
    syms = drive.syms_args(cpath)
    extra += syms
    for dd in (drive.drop_table_defsyms(defsyms2, drive.table_syms()) if syms else defsyms2):
        extra += ["--defsym", dd]
    drive.manifest_append(vs, u.manifest_unit, "%s/%s.c" % (unit, drive.fn_name(vs)), meta["size"],
                          " ".join(extra))
    if not args.no_ledger and ledger.state(unit, vs) != "match":
        ledger.record("match", unit, vs, 0, "permuter (%s, %s draft)" % (how, c["variant"]))
    return "MATCH"


def job(c, args, workdir):
    """levers, then the permuter, then the oracle on the result.  No repo writes."""
    tag = "%s_%s" % (c["unit"], c["vaddr"][2:])
    d = os.path.join(workdir, tag)
    res = {"c": c, "status": "no match", "base": None, "lever": None, "trail": [], "evals": 0,
           "pbase": None, "best": None, "iterations": 0, "lever_s": 0.0, "perm_s": 0.0,
           "text": None, "defsyms": [], "how": "", "meta": None, "note": ""}
    try:
        cfile = draft_file(c, args.drafts, workdir)
        meta = setup_dir(c["unit"], int(c["vaddr"], 16), cfile, d, size=c["size"], tools=tools_once())
        res["meta"] = meta
        with open(cfile, encoding="utf-8") as fh:
            draft = fh.read()
        syms = symtab(meta)
        retail = retail_bytes(c["unit"], int(c["vaddr"], 16), meta["size"])
        inc = os.path.dirname(cfile)
        t0 = time.time()
        text = draft
        if args.levers and args.lever_evals > 0:
            with open(os.path.join(d, "levers.log"), "w", encoding="utf-8", newline="\n") as lg:
                lv = lever_search(meta, syms, draft, retail, inc, args.levers, args.lever_evals, args.beam, lg)
            res.update(base=lv["base"], lever=lv["best"], trail=lv["trail"], evals=lv["evals"])
            text = lv["text"]
            if lv["base"] is None:
                res["note"] = "draft does not build"
                return res
        res["lever_s"] = time.time() - t0
        candidate, how = None, None
        if res["lever"] == 0:
            candidate, how = text, "levers " + " + ".join(res["trail"])
        elif not args.no_permuter and args.budget - res["lever_s"] > 10:
            if text is not draft:
                with open(os.path.join(d, "lever.c"), "w", encoding="utf-8", newline="\n") as fh:
                    fh.write(text)
                preprocess(meta["tools"], os.path.join(d, "lever.c"), os.path.join(d, "base.c"), [inc])
            r = run_permuter(d, args.budget - res["lever_s"], os.path.join(d, "permuter.log"))
            res.update(pbase=r["base"], best=r["best"], iterations=r["iterations"], perm_s=r["seconds"])
            if r["failed"]:
                res["status"] = "permuter failed"
                res["note"] = r["failed"]
                return res
            if r["zero_dir"]:
                with open(os.path.join(r["zero_dir"], "source.c"), encoding="utf-8") as fh:
                    candidate = splice(text, fh.read(), meta["fn"])
                how = "decomp-permuter" + (" after levers " + " + ".join(res["trail"]) if res["trail"] else "")
                if candidate is None:
                    res["note"] = "splice failed"
        if candidate is not None:
            chk = os.path.join(d, "result.c")
            with open(chk, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(candidate)
            clean, defsyms, summ = oracle(c, chk, meta)
            if clean:
                res.update(status="zero", text=candidate, defsyms=defsyms, how=how)
            else:
                res["note"] += " score 0 but oracle %s" % summ
    except Exception as e:      # noqa: BLE001 - one bad body must not stop the run
        res["status"] = "crash"
        res["note"] = ("%s: %s" % (type(e).__name__, e))[:200]
    return res


REPORT_COLS = ["unit", "vaddr", "size", "variant", "words", "base", "lever", "lever_evals", "lever_s",
               "perm_base", "perm_best", "iterations", "perm_s", "status", "how", "note"]


def report_row(r):
    c = r["c"]
    return [c["unit"], c["vaddr"], c["size"], c["variant"], c["words"], r["base"], r["lever"], r["evals"],
            "%.0f" % r["lever_s"], r["pbase"], r["best"], r["iterations"], "%.0f" % r["perm_s"],
            r["status"], r["how"], r["note"].strip()]


def cmd_run(args):
    import concurrent.futures as cf
    cands = pick(args)
    print("%d candidate(s)" % len(cands))
    if args.dry_run:
        for c in cands:
            print(c["unit"], c["vaddr"], c["size"], c["variant"], c["words"])
        return 0
    if not args.no_permuter:
        permuter_dir()
    workdir = os.path.abspath(args.work) if args.work else tempfile.mkdtemp(prefix="ac1_permute_")
    os.makedirs(workdir, exist_ok=True)
    report = os.path.join(workdir, "permute_report.tsv")
    t0 = time.time()
    results = []
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        futs = [ex.submit(job, c, args, workdir) for c in cands]
        for i, f in enumerate(cf.as_completed(futs), 1):
            res = f.result()
            c = res["c"]
            if res["status"] == "zero":
                res["status"] = commit(c, res["meta"], res["text"], res["defsyms"], res["how"], args)
            results.append(res)
            print("[%3d/%d] %-7s %s %4s B  words %-3s base %-4s lever %-4s (%3d ev %3.0f s)  "
                  "perm %-5s best %-5s %5d it %4.0f s  %s %s %s" % (
                      i, len(cands), c["unit"], c["vaddr"], c["size"], c["words"], res["base"], res["lever"],
                      res["evals"], res["lever_s"], res["pbase"], res["best"], res["iterations"],
                      res["perm_s"], res["status"], res["how"], res["note"]))
            sys.stdout.flush()
    wall = time.time() - t0
    hits = [r for r in results if r["status"] == "MATCH"]
    its = sum(r["iterations"] for r in results)
    evs = sum(r["evals"] for r in results)
    with open(report, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\t".join(REPORT_COLS) + "\n")
        for r in sorted(results, key=lambda r: (r["c"]["unit"], r["c"]["vaddr"])):
            fh.write("\t".join("" if x is None else str(x) for x in report_row(r)) + "\n")
    by_lever = sum(1 for r in hits if r["how"].startswith("levers"))
    print("permute: %d/%d matched (%.1f%%; %d by levers alone, %d by the permuter), %d lever compiles + "
          "%d permuter compiles in %.0f s wall = %.1f compiles/s at -j %d; report %s"
          % (len(hits), len(results), 100.0 * len(hits) / len(results) if results else 0, by_lever,
             len(hits) - by_lever, evs, its, wall, (evs + its) / wall if wall else 0, args.jobs, report))
    if hits and not args.no_replay:
        import twins
        print("$ twins.py replay --scope programs")
        twins.main(["replay", "--scope", "programs", "-j", str(args.jobs)])
    return 0


def add_pick_args(ap):
    ap.add_argument("--report", action="append", help="sweep_report.tsv (repeatable)")
    ap.add_argument("--cfile", action="append", metavar="[UNIT:]C", help="near-miss C file (repeatable)")
    ap.add_argument("--max-words", type=int, default=None, help="only reports' drafts with <= N differing words")
    ap.add_argument("--max-ratio", type=float, default=None, help="only words/(size/4) <= R")
    ap.add_argument("--max-size", type=int, default=0, help="only bodies smaller than N bytes")
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--drafts", action="append",
                    help="dir holding <unit>_<ADDR>_<variant>.c drafts (repeatable; a report's own dir is "
                    "always looked in; else the draft is regenerated with sweep.draft)")


def main(argv=None):
    import levers
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("cc")
    p.add_argument("dir")
    p.add_argument("src")
    p.add_argument("-o", dest="out", required=True)
    p = sub.add_parser("setup")
    p.add_argument("unit")
    p.add_argument("vaddr")
    p.add_argument("cfile")
    p.add_argument("dir")
    p = sub.add_parser("pick")
    add_pick_args(p)
    p = sub.add_parser("run")
    add_pick_args(p)
    p.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    p.add_argument("--budget", type=float, default=300, help="seconds per function, both stages (default %(default)s)")
    p.add_argument("--levers", default=levers.DEFAULT_LEVERS, help="lever names, comma-separated ('' = none)")
    p.add_argument("--lever-evals", type=int, default=150, help="compiles for the lever stage (default %(default)s)")
    p.add_argument("--beam", type=int, default=4, help="texts kept per lever depth (default %(default)s)")
    p.add_argument("--no-permuter", action="store_true", help="levers only")
    p.add_argument("--work", help="scratch dir (default a new temp dir)")
    p.add_argument("--dry-run", action="store_true")
    p.add_argument("--no-ledger", action="store_true")
    p.add_argument("--no-replay", action="store_true")
    args = ap.parse_args(argv)
    if args.cmd in ("pick", "run") and not (args.report or args.cfile):
        ap.error("give --report and/or --cfile")
    return {"cc": cmd_cc, "setup": cmd_setup, "pick": cmd_pick, "run": cmd_run}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
