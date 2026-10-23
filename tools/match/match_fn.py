#!/usr/bin/env python3
"""match_fn.py - byte-match oracle for one Armored Core (SLUS-01323) function.

    match_fn.py <vaddr> <src.c> [--bin PATH --base VADDR | --ovl N] [--fn NAME]
                [--defsym NAME=ADDR ...] [--syms FILE.tsv] [--rodata VADDR]
                [--expect-size N] [--json [PATH]] [--keep DIR | --keep-on-fail]
                [-I DIR] [-D MACRO]

Pipeline (what the retail build did, see tools/match/README.md):
    host cpp -E -P  ->  cc1 (GCC 2.7.2 psx build, pinned flags)  ->  maspsx
    (--aspsx-version=2.56)  ->  mipsel GNU as  ->  ELF .o  ->  bytes of <fn>
    ->  compared word by word against the bytes at <vaddr> in the retail binary.

Last line of stdout is exactly one of
    MATCH <fn> <n bytes>
    MISMATCH <fn> <first differing offset>
    ERROR: <reason>
Exit status: 0 MATCH, 1 MISMATCH, 2 toolchain/compile/assemble error, 3 usage.
Only exit status 0 means the function is matched.
"""
import argparse
import difflib
import hashlib
import json
import os
import platform
import re
import shutil
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
VENDOR = os.path.join(HERE, "vendor")

# Retail compiler pin (PsyQ 4.0 CC1PSX == decompals gcc-2.7.2-psx), ASPSX 2.56.
CC1_FLAGS = [
    "-O2", "-mips1", "-mcpu=3000", "-w",
    "-funsigned-char", "-fpeephole", "-ffunction-cse",
    "-fpcc-struct-return", "-fcommon",
    "-msoft-float", "-mgas", "-fgnu-linker",
    "-gcoff", "-G0", "-quiet",
]
ASPSX_VERSION = "2.56"
AS_FLAGS = ["-EL", "-march=r3000", "-mtune=r3000", "-no-pad-sections", "-G0"]
CPP_FLAGS = ["-E", "-P", "-undef", "-nostdinc",
             "-D_LANGUAGE_C", "-D_MIPSEL", "-DMIPSEL", "-Dmips", "-D__mips__",
             "-D__GNUC__=2", "-D__GNUC_MINOR__=7", "-D__psx__"]

EXE_TADDR = 0x80011000
EXE_SECTORS = (39, 121)          # inclusive, raw 2352-byte sectors of the disc image
OVERLAY_BASE = 0x8004ADA0        # all four FDAT program overlays load here

# MIPS REL relocation types
R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16 = 2, 4, 5, 6, 7


class ToolError(Exception):
    pass


def parse_int(s):
    return int(s, 0)


# ----------------------------------------------------------------------------
# toolchain resolution
# ----------------------------------------------------------------------------
def load_toolchain_json():
    p = os.path.join(VENDOR, "toolchain.json")
    if os.path.isfile(p):
        with open(p) as fh:
            return json.load(fh)
    return {}


def which_first(names):
    for n in names:
        p = shutil.which(n)
        if p:
            return p
    return None


def vendor_path(p):
    return p if os.path.isabs(p) else os.path.join(VENDOR, p)


def find_cc1(args, tc):
    for cand in (args.cc1, os.environ.get("AC1_CC1"), tc.get("cc1")):
        if cand:
            cand = vendor_path(cand)
            if os.path.isfile(cand):
                return cand
            raise ToolError("cc1 not found at %s" % cand)
    for cand in ("cc1.exe", "cc1", "cc1.sh"):
        p = os.path.join(VENDOR, cand)
        if os.path.isfile(p):
            return p
    raise ToolError("no cc1: run tools/match/setup.sh (or set AC1_CC1 / --cc1)")


def find_as(args, tc):
    if args.as_:
        return args.as_
    if os.environ.get("AC1_AS"):
        return os.environ["AC1_AS"]
    if tc.get("as") and (os.path.isfile(tc["as"]) or shutil.which(tc["as"])):
        return tc["as"]
    pfx = os.environ.get("MIPS_PREFIX")
    names = ([pfx + "as"] if pfx else []) + ["mipsel-none-elf-as", "mipsel-linux-gnu-as", "mips-linux-gnu-as"]
    p = which_first(names)
    if not p:
        raise ToolError("no mipsel GNU as on PATH (set MIPS_PREFIX or --as)")
    return p


def find_objdump(as_path):
    d, b = os.path.split(as_path)
    cand = os.path.join(d, b.replace("-as", "-objdump"))
    if os.path.isfile(cand):
        return cand
    return which_first(["mipsel-none-elf-objdump", "mipsel-linux-gnu-objdump", "mips-linux-gnu-objdump"])


def find_maspsx(args, tc):
    for cand in (args.maspsx, os.environ.get("AC1_MASPSX"), tc.get("maspsx"),
                 os.path.join(VENDOR, "maspsx", "maspsx.py")):
        if cand:
            cand = vendor_path(cand)
            if os.path.isfile(cand):
                return cand
    raise ToolError("maspsx.py not found: run tools/match/setup.sh (or --maspsx)")


def find_cpp(args, tc):
    for cand in (args.cpp, os.environ.get("AC1_CPP"), tc.get("cpp")):
        if cand:
            return cand
    p = which_first(["cpp", "gcc"])
    if not p:
        raise ToolError("no host cpp/gcc on PATH for preprocessing (--cpp)")
    return p


# ----------------------------------------------------------------------------
# retail binary selection
# ----------------------------------------------------------------------------
def main_checkout():
    """Repo root of the main checkout (worktrees do not carry the gitignored disc_map/overlays)."""
    try:
        out = subprocess.run(["git", "rev-parse", "--git-common-dir"], cwd=ROOT,
                             capture_output=True, text=True, check=True).stdout.strip()
        common = out if os.path.isabs(out) else os.path.abspath(os.path.join(ROOT, out))
        return os.path.dirname(common)
    except Exception:
        return None


def overlays_dirs():
    out = []
    if os.environ.get("AC1_OVERLAYS"):
        out.append(os.environ["AC1_OVERLAYS"])
    out.append(os.path.join(ROOT, "disc_map", "overlays"))
    mc = main_checkout()
    if mc:
        out.append(os.path.join(mc, "disc_map", "overlays"))
    return [d for d in out if os.path.isdir(d)]


def carve_exe(cache_dir):
    disc = os.environ.get("AC1_BIN")
    if not disc or not os.path.isfile(disc):
        raise ToolError("address is in the main exe but AC1_BIN (disc image) is not set/found")
    os.makedirs(cache_dir, exist_ok=True)
    out = os.path.join(cache_dir, "ac1_exe_carved.bin")
    if not os.path.isfile(out):
        data = bytearray()
        with open(disc, "rb") as fh:
            for s in range(EXE_SECTORS[0], EXE_SECTORS[1] + 1):
                fh.seek(s * 2352 + 24)
                data += fh.read(2048)
        if data[:8] != b"PS-X EXE":
            raise ToolError("carved sectors %d-%d of AC1_BIN are not a PS-X EXE" % EXE_SECTORS)
        with open(out, "wb") as fh:
            fh.write(data)
    return out


def pick_bin(args, vaddr):
    """Return (path, VA of file offset 0, label)."""
    if args.bin:
        if args.base is None:
            raise ToolError("--bin needs --base")
        return args.bin, args.base, os.path.basename(args.bin)
    if EXE_TADDR <= vaddr < OVERLAY_BASE:
        p = carve_exe(os.path.join(VENDOR, "cache"))
        return p, EXE_TADDR - 0x800, "SLUS-01323 exe (carved from AC1_BIN)"
    want = args.ovl or 201
    for d in overlays_dirs():
        j = os.path.join(d, "overlays.json")
        if not os.path.isfile(j):
            continue
        with open(j) as fh:
            meta = json.load(fh)
        for o in meta.get("overlays", []):
            if o.get("entry") != want:
                continue
            base = o["load_base"]
            if not (base <= vaddr < base + o["byte_len"]):
                raise ToolError("0x%08x is outside overlay %d (0x%08x..0x%08x)"
                                % (vaddr, want, base, base + o["byte_len"]))
            p = os.path.normpath(os.path.join(d, os.path.basename(o["bin"].replace("\\", "/"))))
            if os.path.isfile(p):
                return p, base, "overlay %d %s" % (want, o.get("name", ""))
    raise ToolError("no carved overlay %d found (disc_map/overlays/overlays.json); use --bin/--base" % want)


# ----------------------------------------------------------------------------
# ELF32 little-endian reader (sections, symbols, REL relocations)
# ----------------------------------------------------------------------------
class Elf:
    def __init__(self, data):
        self.data = data
        if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 1:
            raise ToolError("assembler output is not a 32-bit little-endian ELF")
        self.shoff = struct.unpack_from("<I", data, 0x20)[0]
        self.shentsize, self.shnum, self.shstrndx = struct.unpack_from("<HHH", data, 0x2e)
        self.sections = []
        for i in range(self.shnum):
            off = self.shoff + i * self.shentsize
            (name, typ, flags, addr, offset, size, link, info, align,
             entsize) = struct.unpack_from("<IIIIIIIIII", data, off)
            self.sections.append(dict(idx=i, name_off=name, type=typ, flags=flags, addr=addr,
                                      offset=offset, size=size, link=link, info=info, entsize=entsize))
        shstr = self.sections[self.shstrndx]
        for s in self.sections:
            s["name"] = self.cstr(shstr["offset"] + s["name_off"])
        self.by_name = {s["name"]: s for s in self.sections}
        sym = next((s for s in self.sections if s["type"] == 2), None)
        self.symbols = []
        if sym:
            strtab = self.sections[sym["link"]]
            for i in range(sym["size"] // 16):
                st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack_from(
                    "<IIIBBH", data, sym["offset"] + i * 16)
                self.symbols.append(dict(idx=i, name=self.cstr(strtab["offset"] + st_name), value=st_value,
                                         size=st_size, type=st_info & 0xf, bind=st_info >> 4, shndx=st_shndx))

    def cstr(self, off):
        end = self.data.index(b"\0", off)
        return self.data[off:end].decode("latin-1")

    def section_bytes(self, name):
        s = self.by_name.get(name)
        if not s or s["type"] == 8:   # NOBITS
            return b""
        return self.data[s["offset"]:s["offset"] + s["size"]]

    def rels(self, secname):
        r = self.by_name.get(".rel" + secname)
        out = []
        if r and r["type"] == 9:   # SHT_REL
            for i in range(r["size"] // 8):
                off, info = struct.unpack_from("<II", self.data, r["offset"] + i * 8)
                out.append(dict(offset=off, sym=info >> 8, type=info & 0xff))
        return out


# ----------------------------------------------------------------------------
# symbol table for external references
# ----------------------------------------------------------------------------
def load_symbols(args):
    syms = {}
    files = list(args.syms or [])
    default_tsv = os.path.join(ROOT, "ac1_markup.tsv")
    if not files and os.path.isfile(default_tsv) and not args.no_markup:
        files.append(default_tsv)
    for f in files:
        with open(f, encoding="utf-8", errors="replace") as fh:
            for line in fh:
                parts = line.rstrip("\n").split("\t")
                if len(parts) >= 3 and re.fullmatch(r"[0-9a-fA-F]{6,8}", parts[1]):
                    syms.setdefault(parts[2], int(parts[1], 16))          # ac1_markup.tsv: kind addr name ...
                elif len(parts) >= 2 and re.fullmatch(r"(0x)?[0-9a-fA-F]{6,8}", parts[0]):
                    syms.setdefault(parts[1], int(parts[0], 16))          # plain "addr<TAB>name"
    for d in args.defsym or []:
        if "=" not in d:
            raise ToolError("--defsym wants NAME=ADDR, got %r" % d)
        n, v = d.split("=", 1)
        syms[n] = parse_int(v)          # explicit --defsym always wins
    return syms


# ----------------------------------------------------------------------------
# compile pipeline
# ----------------------------------------------------------------------------
def run(cmd, cwd, log):
    log.append("$ " + " ".join(cmd))
    try:
        p = subprocess.run(cmd, cwd=cwd, stdin=subprocess.DEVNULL, capture_output=True, text=True)
    except OSError as e:
        raise ToolError("cannot run %s: %s" % (cmd[0], e))
    if p.stderr.strip():
        log.append(p.stderr.rstrip())
    if p.returncode != 0:
        raise ToolError("%s failed (exit %d):\n%s%s" % (os.path.basename(cmd[0]), p.returncode, p.stdout, p.stderr))
    return p.stdout


def fix_rodata_align(asm_text):
    """cc1 writes `.align 3` in front of every jump table; the PsyQ linker packed retail
    objects 4-aligned, so the 8-byte alignment never shows in the image.  Turn `.align 3`
    into `.align 2` inside .rdata/.rodata only (text is untouched)."""
    out, sec, n = [], None, 0
    for line in asm_text.split("\n"):
        s = line.strip()
        if s in (".text", ".data", ".rdata", ".sdata", ".bss", ".sbss", ".rodata"):
            sec = s
        elif s.startswith(".section"):
            parts = s.split()
            sec = parts[1].rstrip(",") if len(parts) > 1 else None
        elif sec in (".rdata", ".rodata") and re.fullmatch(r"\.align\s+3", s):
            line = line.replace("3", "2", 1)
            n += 1
        out.append(line)
    return "\n".join(out), n


def cc1_command(cc1):
    """A .sh wrapper (WSL route) is run through bash; a native binary directly."""
    if cc1.lower().endswith(".sh"):
        return ["bash", cc1]
    return [cc1]


def compile_to_object(args, tools, src, work, log):
    """cpp -> cc1 -> (align fix) -> maspsx -> as.  Returns the .o path and cc1's asm."""
    src_abs = os.path.abspath(src)
    shutil.copyfile(src_abs, os.path.join(work, "fn.c"))
    incs = []
    for d in [os.path.dirname(src_abs), os.path.join(HERE, "include")] + (args.include or []):
        incs += ["-I", d]
    defs = ["-D" + m for m in (args.define or [])]
    cpp = tools["cpp"]
    cpp_cmd = [cpp] + (["-x", "c"] if "gcc" in os.path.basename(cpp) else []) + \
        CPP_FLAGS + defs + incs + ["fn.c", "-o", "fn.i"]
    run(cpp_cmd, work, log)

    # relative file names on purpose: a WSL wrapper then needs no path translation
    run(cc1_command(tools["cc1"]) + ["fn.i"] + CC1_FLAGS + ["-o", "fn.s"], work, log)
    with open(os.path.join(work, "fn.s"), encoding="utf-8", errors="replace") as fh:
        asm = fh.read()
    if not args.no_rodata_align_fix:
        asm, nfix = fix_rodata_align(asm)
        if nfix:
            log.append("rodata align fix: %d x `.align 3` -> `.align 2`" % nfix)
    with open(os.path.join(work, "fn.fixed.s"), "w", newline="\n") as fh:
        fh.write(asm)

    mas_out = run([sys.executable, tools["maspsx"], "--aspsx-version=" + args.aspsx_version,
                   "fn.fixed.s"], work, log)
    with open(os.path.join(work, "fn.maspsx.s"), "w", newline="\n") as fh:
        fh.write(mas_out)
    run([tools["as"]] + AS_FLAGS + ["-o", "fn.o", "fn.maspsx.s"], work, log)
    return os.path.join(work, "fn.o"), asm


# ----------------------------------------------------------------------------
# function extraction + relocation handling
# ----------------------------------------------------------------------------
def pick_function(elf, name):
    text = elf.by_name.get(".text")
    if not text:
        raise ToolError("object has no .text section")
    funcs = [s for s in elf.symbols if s["shndx"] == text["idx"] and s["type"] == 2 and s["name"]]
    if not funcs:
        funcs = [s for s in elf.symbols if s["shndx"] == text["idx"] and s["bind"] == 1 and s["name"]]
    if name:
        f = next((s for s in funcs if s["name"] == name), None)
        if not f:
            raise ToolError("function %r not in object (have: %s)"
                            % (name, ", ".join(s["name"] for s in funcs) or "none"))
    elif len(funcs) == 1:
        f = funcs[0]
    elif not funcs:
        raise ToolError("no function symbol in .text")
    else:
        raise ToolError("object defines several functions (%s): pass --fn" % ", ".join(s["name"] for s in funcs))
    start = f["value"]
    size = f["size"]
    if size == 0:
        nxt = sorted(s["value"] for s in funcs if s["value"] > start)
        size = (nxt[0] if nxt else text["size"]) - start
    return f["name"], start, size


def sext16(v):
    return v - 0x10000 if v & 0x8000 else v


def resolve_relocs(elf, fn_start, fn_size, text_vaddr, symtab, rodata_vaddr, data_vaddr, targets=None):
    """Apply every relocation inside the function whose target address is known
    (--defsym / symbol TSV / own .text / --rodata / --data); mask the immediate
    field of the rest.  Returns (fn_bytes, {fn_offset: mask}, resolved, unresolved).
    A list passed as `targets` gets one (type, symbol, effective address or None)
    tuple per relocation, for the full-link readiness audit."""
    text = bytearray(elf.section_bytes(".text"))
    rels = sorted(elf.rels(".text"), key=lambda r: r["offset"])
    masks, resolved, unresolved = {}, [], []
    sec_vaddr = {".text": text_vaddr}
    if rodata_vaddr is not None:
        sec_vaddr[".rodata"] = rodata_vaddr
        sec_vaddr[".rdata"] = rodata_vaddr
    if data_vaddr is not None:
        sec_vaddr[".data"] = data_vaddr

    def sym_value(sym):
        if sym["type"] == 3:                          # STT_SECTION
            secname = elf.sections[sym["shndx"]]["name"]
            return sec_vaddr.get(secname), secname
        if sym["shndx"] == 0:                         # undefined -> external
            return symtab.get(sym["name"]), sym["name"]
        secname = elf.sections[sym["shndx"]]["name"] if sym["shndx"] < len(elf.sections) else "?"
        base = sec_vaddr.get(secname)
        return (base + sym["value"]) if base is not None else None, sym["name"]

    def word(off):
        return struct.unpack_from("<I", text, off)[0]

    def put(off, w):
        struct.pack_into("<I", text, off, w & 0xffffffff)

    def note(kind, label, addr):
        if targets is not None:
            targets.append((kind, label, None if addr is None else addr & 0xffffffff))

    for i, r in enumerate(rels):
        off = r["offset"]
        in_fn = fn_start <= off < fn_start + fn_size
        if not in_fn:
            continue
        sym = elf.symbols[r["sym"]]
        val, label = sym_value(sym)
        t = r["type"]
        if t == R_MIPS_26:
            if val is None:
                masks[off] = 0x03ffffff
                unresolved.append(label)
                note("26", label, None)
            else:
                a = (word(off) & 0x03ffffff) << 2
                put(off, (word(off) & 0xfc000000) | (((val + a) >> 2) & 0x03ffffff))
                resolved.append(label)
                note("26", label, val + a)
        elif t == R_MIPS_HI16:
            lo = next((q for q in rels[i + 1:] if q["type"] == R_MIPS_LO16 and q["sym"] == r["sym"]), None)
            if val is None or lo is None:
                masks[off] = 0xffff
                unresolved.append(label if lo else label + " (unpaired %hi)")
                note("hi16", label, None)
            else:
                ahl = ((word(off) & 0xffff) << 16) + sext16(word(lo["offset"]) & 0xffff)
                v = (val + ahl) & 0xffffffff
                put(off, (word(off) & 0xffff0000) | (((v + 0x8000) >> 16) & 0xffff))
                resolved.append(label)
                note("hi16", label, v)
        elif t == R_MIPS_LO16:
            if val is None:
                masks[off] = 0xffff
                unresolved.append(label)
                note("lo16", label, None)
            else:
                v = (val + sext16(word(off) & 0xffff)) & 0xffffffff
                put(off, (word(off) & 0xffff0000) | (v & 0xffff))
                resolved.append(label)
                note("lo16", label, v)
        elif t == R_MIPS_32:
            if val is None:
                masks[off] = 0xffffffff
                unresolved.append(label)
                note("32", label, None)
            else:
                note("32", label, val + word(off))
                put(off, (val + word(off)) & 0xffffffff)
                resolved.append(label)
        else:
            masks[off] = 0xffffffff
            unresolved.append("%s (reloc type %d)" % (label, t))
    fn_bytes = bytes(text[fn_start:fn_start + fn_size])
    fn_masks = {off - fn_start: m for off, m in masks.items()}
    return fn_bytes, fn_masks, resolved, unresolved


# ----------------------------------------------------------------------------
# compare + report
# ----------------------------------------------------------------------------
def disasm(objdump, blob, vaddr, work, tag):
    p = os.path.join(work, tag + ".bin")
    with open(p, "wb") as fh:
        fh.write(blob)
    if not objdump:
        return {vaddr + i: ".word 0x%08x" % struct.unpack_from("<I", blob, i)[0]
                for i in range(0, len(blob) - 3, 4)}
    out = subprocess.run([objdump, "-D", "-b", "binary", "-m", "mips:isa32", "-EL",
                          "--adjust-vma=0x%x" % vaddr, p], capture_output=True, text=True).stdout
    lines = {}
    for line in out.splitlines():
        m = re.match(r"^\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*)$", line)
        if m:
            lines[int(m.group(1), 16)] = m.group(3).strip().replace("	", " ")
    return lines


def compare(ours, retail, masks):
    n = min(len(ours), len(retail))
    diffs = []
    for off in range(0, n, 4):
        a = struct.unpack_from("<I", ours, off)[0]
        b = struct.unpack_from("<I", retail, off)[0]
        m = masks.get(off, 0)
        if (a & ~m) != (b & ~m):
            diffs.append(off)
    first = diffs[0] if diffs else None
    if first is None and len(retail) < len(ours):
        first = len(retail)
    return first, diffs


def aligned_words(ours, retail):
    """Words of `retail` that difflib pairs with an equal word of `ours`, in order: a
    closeness that, unlike diff_words, survives an inserted or missing instruction."""
    a = list(struct.unpack_from("<%dI" % (len(ours) // 4), ours))
    b = list(struct.unpack_from("<%dI" % (len(retail) // 4), retail))
    return sum(m.size for m in difflib.SequenceMatcher(None, a, b, autojunk=False).get_matching_blocks())


def side_by_side(ours, retail, vaddr, diffs, masks, objdump, work, context):
    ld = disasm(objdump, retail, vaddr, work, "retail")
    rd = disasm(objdump, ours, vaddr, work, "ours")
    n = max(len(ours), len(retail))
    if context is None or not diffs:
        show = set(range(0, n, 4))
    else:
        show = set()
        for d in diffs:
            for k in range(d - 4 * context, d + 4 * context + 4, 4):
                show.add(k)
    rows = ["  %-8s  %-34s | %s" % ("addr", "retail", "ours")]
    last = None
    for off in range(0, n, 4):
        if off not in show:
            continue
        if last is not None and off != last + 4:
            rows.append("  ...")
        last = off
        a = vaddr + off
        left = ld.get(a, "--") if off < len(retail) else "<end of retail slice>"
        right = rd.get(a, "--") if off < len(ours) else "<end of ours>"
        mark = "!" if off in diffs else ("m" if off in masks else " ")
        rows.append("%s %08x  %-34s | %s" % (mark, a, left, right))
    return rows


def make_parser():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("vaddr", type=parse_int, help="runtime address of the retail function (0x...)")
    ap.add_argument("src", help="C file containing the function")
    ap.add_argument("--bin", help="retail binary to compare against (default: overlay/exe chosen by address)")
    ap.add_argument("--base", type=parse_int, help="VA of byte 0 of --bin")
    ap.add_argument("--ovl", type=int, help="overlay number when the address is in overlay space (default 201)")
    ap.add_argument("--fn", help="function name in the object (default: the only function)")
    ap.add_argument("--defsym", action="append", metavar="NAME=ADDR", help="address of an external symbol (repeatable)")
    ap.add_argument("--syms", action="append", metavar="FILE", help="TSV of addresses (default: ac1_markup.tsv)")
    ap.add_argument("--no-markup", action="store_true", help="do not read ac1_markup.tsv for symbol addresses")
    ap.add_argument("--rodata", type=parse_int, help="retail VA of this object's .rodata (jump tables, const data)")
    ap.add_argument("--data", type=parse_int, help="retail VA of this object's .data")
    ap.add_argument("--expect-size", type=parse_int, help="retail function size in bytes; any other size is a MISMATCH")
    ap.add_argument("--json", nargs="?", const="-", metavar="PATH", help="also emit the facts as JSON (to PATH, or stdout)")
    ap.add_argument("--keep", metavar="DIR", help="keep the work dir (fn.i, fn.s, fn.maspsx.s, fn.o) here")
    ap.add_argument("--keep-on-fail", action="store_true",
                    help="keep the temp work dir on MISMATCH/ERROR (path printed); also env AC1_MATCH_KEEP_ON_FAIL=1")
    ap.add_argument("-I", dest="include", action="append", help="extra include dir for cpp")
    ap.add_argument("-D", dest="define", action="append", help="extra macro for cpp")
    ap.add_argument("--context", type=int, default=None, help="diff context lines (default: whole function)")
    ap.add_argument("--no-rodata-align-fix", action="store_true", help="keep cc1's `.align 3` before jump tables")
    ap.add_argument("--aspsx-version", default=ASPSX_VERSION)
    ap.add_argument("--cc1")
    ap.add_argument("--as", dest="as_")
    ap.add_argument("--maspsx")
    ap.add_argument("--cpp")
    ap.add_argument("-q", "--quiet", action="store_true", help="only the final line (and JSON)")
    return ap


def main(argv=None):
    args = make_parser().parse_args(argv)

    facts = {"result": None, "fn": None, "vaddr": "0x%08x" % args.vaddr, "src": args.src,
             "bytes": None, "first_diff_offset": None, "diff_words": 0, "aligned_words": None,
             "masked_words": 0, "unresolved_symbols": [], "resolved_symbols": [],
             "bin": None, "bin_label": None, "bin_offset": None, "cc1": None,
             "aspsx_version": args.aspsx_version, "cc1_flags": CC1_FLAGS, "as_flags": AS_FLAGS,
             "error": None}
    log = []
    fn_name = None
    status = 2
    work = None
    keep_on_fail = args.keep_on_fail or os.environ.get("AC1_MATCH_KEEP_ON_FAIL", "") not in ("", "0")
    try:
        if not os.path.isfile(args.src):
            raise ToolError("source file not found: %s" % args.src)
        with open(args.src, "rb") as fh:
            facts["src_sha1"] = hashlib.sha1(fh.read()).hexdigest()
        tc = load_toolchain_json()
        tools = {"cc1": find_cc1(args, tc), "as": find_as(args, tc), "maspsx": find_maspsx(args, tc),
                 "cpp": find_cpp(args, tc)}
        tools["objdump"] = find_objdump(tools["as"])
        facts["cc1"] = tools["cc1"]
        bin_path, bin_base, label = pick_bin(args, args.vaddr)
        facts["bin"], facts["bin_label"] = bin_path, label

        work = args.keep or tempfile.mkdtemp(prefix="match_fn_")
        os.makedirs(work, exist_ok=True)
        obj, _asm = compile_to_object(args, tools, args.src, work, log)
        with open(obj, "rb") as fh:
            elf = Elf(fh.read())
        fn_name, fn_start, fn_size = pick_function(elf, args.fn)
        facts["fn"] = fn_name
        facts["bytes"] = fn_size
        if fn_size == 0 or fn_size % 4:
            raise ToolError("function %s has size %d" % (fn_name, fn_size))
        text_vaddr = args.vaddr - fn_start
        targets = []
        ours, masks, resolved, unresolved = resolve_relocs(elf, fn_start, fn_size, text_vaddr,
                                                           load_symbols(args), args.rodata, args.data,
                                                           targets)
        facts["relocs"] = [{"type": k, "sym": n, "addr": None if a is None else "0x%08x" % a}
                           for k, n, a in targets]
        facts["masked_words"] = len(masks)
        facts["unresolved_symbols"] = sorted(set(unresolved))
        facts["resolved_symbols"] = sorted(set(resolved))

        off = args.vaddr - bin_base
        with open(bin_path, "rb") as fh:
            fh.seek(0, 2)
            bin_len = fh.tell()
            if off < 0 or off >= bin_len:
                raise ToolError("0x%08x is outside %s (base 0x%08x, %d bytes)"
                                % (args.vaddr, bin_path, bin_base, bin_len))
            fh.seek(off)
            retail = fh.read(fn_size)
        facts["bin_offset"] = off

        first, diffs = compare(ours, retail, masks)
        facts["diff_words"] = len(diffs)
        if diffs or (args.expect_size is not None and args.expect_size != fn_size):
            with open(bin_path, "rb") as fh:
                fh.seek(off)
                facts["aligned_words"] = aligned_words(ours, fh.read(args.expect_size or fn_size))
        else:
            facts["aligned_words"] = fn_size // 4
        size_bad = args.expect_size is not None and args.expect_size != fn_size
        if not args.quiet:
            print("fn %s: %d bytes from %s, compared with %s @ 0x%08x (file offset 0x%x)"
                  % (fn_name, fn_size, os.path.basename(args.src), label, args.vaddr, off))
            if masks:
                print("WARNING: %d word(s) masked, unresolved symbol(s): %s  (pass --defsym NAME=ADDR / --rodata VA)"
                      % (len(masks), ", ".join(facts["unresolved_symbols"])))
            if resolved:
                print("resolved symbol(s): %s" % ", ".join(facts["resolved_symbols"]))
            if first is not None or size_bad:
                print("\n".join(side_by_side(ours, retail, args.vaddr, set(diffs), masks,
                                             tools["objdump"], work, args.context)))
                print("%d differing word(s)%s" % (len(diffs), "; work dir: " + work
                                                  if args.keep or keep_on_fail else ""))
            if size_bad:
                print("size: ours %d bytes, expected %d" % (fn_size, args.expect_size))
        if first is None and not size_bad:
            facts["result"] = "MATCH"
            status = 0
        else:
            facts["result"] = "MISMATCH"
            facts["first_diff_offset"] = first if first is not None else fn_size
            status = 1
    except ToolError as e:
        facts["result"] = "ERROR"
        facts["error"] = str(e)
        status = 2
    finally:
        # The auto-created temp dir always goes, also on MISMATCH/ERROR or an unexpected
        # exception; only --keep DIR or --keep-on-fail (on a miss) leaves it behind.
        if work and not args.keep and os.path.isdir(work):
            if status != 0 and keep_on_fail:
                print("kept work dir: " + work, file=sys.stderr)
            else:
                shutil.rmtree(work, ignore_errors=True)

    if not args.quiet and log and status != 0:
        print("--- toolchain log ---")
        print("\n".join(log))
    if args.json:
        js = json.dumps(facts, indent=2, sort_keys=True)
        if args.json == "-":
            print(js)
        else:
            with open(args.json, "w") as fh:
                fh.write(js + "\n")
    if status == 0:
        print("MATCH %s %d" % (fn_name, facts["bytes"]))
    elif status == 1:
        print("MISMATCH %s 0x%x" % (fn_name, facts["first_diff_offset"]))
    else:
        print("ERROR: %s" % facts["error"])
    return status


if __name__ == "__main__":
    sys.exit(main())
