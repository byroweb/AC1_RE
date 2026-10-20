#!/usr/bin/env python3
"""ctx.py - shared-type context for the AC1 matching loop.

include/ac1/*.h hold our struct layouts; include/ac1/symbols.tsv maps addresses to
names and C types.  This tool keeps the two in step and feeds them to m2c.

    ctx.py gen                    regenerate include/ac1/symbols.h from symbols.tsv
    ctx.py check                  symbols.h up to date, headers pass the GCC 2.7.2
                                  dialect checks, every struct size assert holds,
                                  m2c parses the context
    ctx.py context [-o FILE] [--proto DECL ...]
                                  cpp include/ac1/ac1.h with -DM2C into one file for
                                  `m2c.py --context FILE` (default build/ac1_ctx.c)
    ctx.py m2c VADDR [LEN] [--unit U] [--proto DECL ...] [--raw] [-o FILE]
                                  decompile one function with the context.  LEN
                                  defaults to the size in tools/loop/functions.tsv.
                                  Before m2c runs, jal targets and lui/%lo address
                                  pairs that land in symbols.tsv (or the exe names
                                  in ac1_markup.tsv) are rewritten to symbols, so
                                  m2c prints g_ui_pool_k2[i].light instead of
                                  *(s16 *)0x801A370E.  A switch's table load (lui;
                                  addu idx; lw; jr) becomes jtbl_<ADDR>, the case
                                  targets get labels and the table is appended as
                                  .rodata from the retail image, so m2c prints the
                                  switch instead of failing.  --raw skips the
                                  context and the rewrite (the old overlay2c.sh
                                  output).

--proto gives the function itself a signature, e.g.
    ctx.py m2c 0x800AD868 --unit 201 --proto "void func_800AD868(UiObj *o);"

match_fn.py reads the same table:  --syms include/ac1/symbols.tsv --syms ac1_markup.tsv
(passing any --syms turns off the implicit ac1_markup.tsv, so name both).  A source
gets the types with `#include "ac1/ac1.h"`; tools/match/include/ac1/ac1.h forwards
there, so match_fn.py needs no -I.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
INC = os.path.join(ROOT, "include")
AC1 = os.path.join(INC, "ac1")
SYMS_TSV = os.path.join(AC1, "symbols.tsv")
SYMS_H = os.path.join(AC1, "symbols.h")
UMBRELLA = os.path.join(AC1, "ac1.h")
MARKUP = os.path.join(ROOT, "ac1_markup.tsv")
FUNCTIONS = os.path.join(ROOT, "tools", "loop", "functions.tsv")
OBJDUMP2M2C = os.path.join(ROOT, "tools", "ghidra", "objdump2m2c.py")
DEFAULT_CTX = os.path.join(ROOT, "build", "ac1_ctx.c")

# same dialect switches match_fn.py gives the host cpp, plus M2C so the size
# asserts (sizeof in an array bound) drop out of the context
CPP_FLAGS = ["-E", "-P", "-undef", "-nostdinc",
             "-D_LANGUAGE_C", "-D_MIPSEL", "-DMIPSEL", "-Dmips", "-D__mips__",
             "-D__GNUC__=2", "-D__GNUC_MINOR__=7", "-D__psx__"]

PRIM = {"u8": 1, "s8": 1, "u16": 2, "s16": 2, "u32": 4, "s32": 4, "void": 1}


class CtxError(Exception):
    pass


# ----------------------------------------------------------------------------
# symbols.tsv
# ----------------------------------------------------------------------------
def load_syms(path=SYMS_TSV):
    rows, seen = [], {}
    with open(path, encoding="utf-8") as fh:
        for n, line in enumerate(fh, 1):
            line = line.rstrip("\r\n")
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) != 4:
                raise CtxError("%s:%d: want 4 tab-separated columns, got %d" % (path, n, len(parts)))
            addr, name, ctype, unit = (p.strip() for p in parts)
            if not re.fullmatch(r"0x[0-9A-Fa-f]{8}", addr):
                raise CtxError("%s:%d: address %r is not 0x + 8 hex digits" % (path, n, addr))
            if not re.fullmatch(r"[A-Za-z_]\w*", name):
                raise CtxError("%s:%d: bad name %r" % (path, n, name))
            if name in seen:
                raise CtxError("%s:%d: %s already defined on line %d" % (path, n, name, seen[name]))
            seen[name] = n
            rows.append(dict(addr=int(addr, 16), name=name, ctype=ctype, unit=unit_key(unit), line=n))
    return rows


def unit_key(u):
    u = str(u).strip().lower()
    m = re.fullmatch(r"(?:ovl)?(20[1-4])", u)
    if m:
        return m.group(1)
    m = re.fullmatch(r"fdat0*(\d+)", u)
    if m:
        return "fdat%03d" % int(m.group(1))
    if u in ("exe", "main", ""):
        return "exe"
    raise CtxError("bad unit %r" % u)


def is_function(ctype):
    return "(" in ctype


def declaration(name, ctype):
    """C declaration for one symbols.tsv row (without the trailing ';')."""
    if is_function(ctype):
        i = ctype.index("(")
        ret, params = ctype[:i].rstrip(), ctype[i:]
        sep = "" if ret.endswith("*") else " "
        return "%s%s%s%s" % (ret, sep, name, params)
    if "[" in ctype:
        i = ctype.index("[")
        base, dims = ctype[:i].rstrip(), ctype[i:]
        sep = "" if base.endswith("*") else " "
        return "%s%s%s%s" % (base, sep, name, dims)
    sep = "" if ctype.endswith("*") else " "
    return "%s%s%s" % (ctype, sep, name)


def render_symbols_h(rows):
    out = ["/* include/ac1/symbols.h - GENERATED by tools/match/ctx.py gen from",
           " * include/ac1/symbols.tsv.  Do not edit; edit the TSV and regenerate. */",
           "#ifndef AC1_SYMBOLS_H",
           "#define AC1_SYMBOLS_H",
           "",
           '#include "types.h"',
           '#include "uiobj.h"',
           '#include "entity.h"',
           '#include "projectile.h"',
           '#include "mission.h"']
    unit = None
    for r in rows:
        if r["unit"] != unit:
            unit = r["unit"]
            out += ["", "/* --- %s --- */" % (unit if unit in ("exe",) or unit.startswith("fdat") else "overlay " + unit)]
        storage = "" if is_function(r["ctype"]) else "extern "
        out.append("%s%s;%s/* 0x%08X */" % (storage, declaration(r["name"], r["ctype"]),
                                           " " * max(1, 56 - len(storage + declaration(r["name"], r["ctype"]))),
                                           r["addr"]))
    out += ["", "#endif", ""]
    return "\n".join(out)


# ----------------------------------------------------------------------------
# type sizes (for resolving an address inside a symbol to symbol + offset)
# ----------------------------------------------------------------------------
def struct_sizes():
    sizes = dict(PRIM)
    for fn in sorted(os.listdir(AC1)):
        if fn.endswith(".h"):
            with open(os.path.join(AC1, fn), encoding="utf-8") as fh:
                for m in re.finditer(r"AC1_ASSERT_SIZE\(\s*(\w+)\s*,\s*(0x[0-9A-Fa-f]+|\d+)\s*\)", fh.read()):
                    sizes[m.group(1)] = int(m.group(2), 0)
    return sizes


def type_size(ctype, sizes):
    """Bytes covered by a data symbol; None = open-ended (T[]), 0 = function."""
    if is_function(ctype):
        return 0
    m = re.fullmatch(r"\s*([A-Za-z_]\w*)\s*(\**)\s*((?:\[\s*\w*\s*\])*)\s*", ctype)
    if not m:
        return 1
    base, stars, dims = m.groups()
    elem = 4 if stars else sizes.get(base, 1)
    n = 1
    for d in re.findall(r"\[\s*(\w*)\s*\]", dims):
        if d == "":
            return None
        n *= int(d, 0)
    return elem * n


# ----------------------------------------------------------------------------
# tools
# ----------------------------------------------------------------------------
def find_cpp():
    for cand in (os.environ.get("AC1_CPP"), "cpp", "mipsel-none-elf-cpp", "mipsel-linux-gnu-cpp"):
        if cand and (os.path.isfile(cand) or shutil.which(cand)):
            return cand
    raise CtxError("no C preprocessor (set AC1_CPP or put cpp on PATH)")


def mips_tool(name):
    pref = os.environ.get("MIPS_PREFIX")
    for p in ([pref] if pref else []) + ["mipsel-none-elf-", "mipsel-linux-gnu-"]:
        if shutil.which(p + name):
            return p + name
    raise CtxError("no %s (set MIPS_PREFIX)" % name)


def m2c_cmd():
    m2c = os.environ.get("M2C_DIR") or os.path.expanduser("~/Applications/m2c")
    for py in (os.path.join(m2c, ".venv", "Scripts", "python.exe"), os.path.join(m2c, ".venv", "bin", "python")):
        if os.path.isfile(py):
            return [py, os.path.join(m2c, "m2c.py")]
    if os.path.isfile(os.path.join(m2c, "m2c.py")):
        return [sys.executable, os.path.join(m2c, "m2c.py")]
    raise CtxError("m2c not found (set M2C_DIR)")


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, errors="replace", **kw)
    if r.returncode:
        raise CtxError("%s failed (%d):\n%s%s" % (os.path.basename(cmd[0]), r.returncode, r.stdout, r.stderr))
    return r.stdout


# ----------------------------------------------------------------------------
# context
# ----------------------------------------------------------------------------
def make_context(protos=(), extra_defines=("M2C",)):
    if not os.path.isfile(SYMS_H) or open(SYMS_H, encoding="utf-8").read() != render_symbols_h(load_syms()):
        raise CtxError("include/ac1/symbols.h is stale; run tools/match/ctx.py gen")
    out = run([find_cpp()] + CPP_FLAGS + ["-D" + d for d in extra_defines] + ["-I", INC, UMBRELLA])
    lines = [l for l in out.splitlines() if l.strip() and l.strip() != ";"]
    for p in protos:
        p = p.strip()
        lines.append(p if p.endswith(";") else p + ";")
    return "/* m2c context generated by tools/match/ctx.py from include/ac1 */\n" + "\n".join(lines) + "\n"


def write_context(path, protos=()):
    d = os.path.dirname(os.path.abspath(path))
    os.makedirs(d, exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(make_context(protos))
    return path


# ----------------------------------------------------------------------------
# symbolising the asm before m2c
# ----------------------------------------------------------------------------
class SymbolMap:
    """Address -> name for one unit: symbols.tsv rows of that unit and of the exe,
    exe function names from ac1_markup.tsv, and function starts of that unit from
    tools/loop/functions.tsv (named func_XXXXXXXX)."""

    def __init__(self, unit):
        self.unit = unit
        sizes = struct_sizes()
        rows = [r for r in load_syms() if r["unit"] in (unit, "exe")]
        self.data = []          # (start, size or None, name)
        self.funcs = {}
        for r in rows:
            sz = type_size(r["ctype"], sizes)
            if sz == 0:
                self.funcs[r["addr"]] = r["name"]
            else:
                self.data.append((r["addr"], sz, r["name"]))
        if os.path.isfile(MARKUP):
            with open(MARKUP, encoding="utf-8", errors="replace") as fh:
                for line in fh:
                    p = line.rstrip("\r\n").split("\t")
                    if len(p) >= 3 and p[0] == "F" and re.fullmatch(r"[0-9a-fA-F]{8}", p[1]):
                        a = int(p[1], 16)
                        if 0x80010000 <= a < 0x8004ADA0:
                            self.funcs.setdefault(a, p[2])
        fu = {"exe": "exe"}.get(unit, "ovl" + unit if unit.isdigit() else unit)
        if os.path.isfile(FUNCTIONS):
            with open(FUNCTIONS, encoding="utf-8") as fh:
                for line in fh:
                    p = line.rstrip("\r\n").split("\t")
                    if len(p) >= 4 and p[0] in (fu, "exe") and p[1].startswith("0x"):
                        a = int(p[1], 16)
                        nm = p[3] if p[0] == fu or a < 0x8004ADA0 else None
                        if nm:
                            self.funcs.setdefault(a, nm)
        # open-ended arrays run to the next data symbol
        self.data.sort()
        fixed = []
        for i, (a, sz, n) in enumerate(self.data):
            if sz is None:
                nxt = [b for b, _, _ in self.data if b > a]
                sz = (min(nxt) - a) if nxt else 0x10000
            fixed.append((a, sz, n))
        self.data = fixed

    def func(self, addr):
        return self.funcs.get(addr)

    def lookup(self, addr):
        """-> 'name' or 'name+0xOFF' for a data address, or a function name."""
        if addr in self.funcs:
            return self.funcs[addr]
        best = None
        for a, sz, n in self.data:
            if a <= addr < a + sz and (best is None or sz < best[1]):
                best = (a, sz, n)
        if best is None:
            # unknown RAM / data address: the usual decomp placeholder name
            return "D_%08X" % addr if 0x80010000 <= addr < 0x80200000 else None
        off = addr - best[0]
        return best[2] if off == 0 else "%s+0x%X" % (best[2], off)


MEM_OPS = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr", "sb", "sh", "sw", "swl", "swr", "lwc2", "swc2"}
NO_DEST = {"sb", "sh", "sw", "swl", "swr", "swc2", "mult", "multu", "div", "divu", "mthi", "mtlo",
           "jr", "j", "b", "beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez", "nop",
           "ctc2", "mtc2", "break", "syscall"}
INSN_RE = re.compile(r"^(\s+)(\S+)\s*(.*)$")


def jr_loads(parsed, lines):
    """Line indices of `lw rD, ...` whose rD reaches a `jr rD` (not $ra) in the same
    straight-line run: the load of a switch's jump-table entry."""
    out = set()
    for i, p in enumerate(parsed):
        if not p or p[0] != "lw" or len(p[1]) != 2:
            continue
        reg = p[1][0]
        for j in range(i + 1, min(i + 8, len(parsed))):
            q = parsed[j]
            if q is None:
                if lines[j].rstrip().endswith(":"):
                    break
                continue
            if q[0] == "jr":
                if q[1] and q[1][0] == reg and reg != "$ra":
                    out.add(i)
                break
            if q[1] and q[1][0] == reg and q[0] not in NO_DEST:
                break
    return out


def symbolise(asm, smap, jtbls=()):
    """Rewrite jal func_xxxxxxxx and lui/%lo pairs that hit a known symbol.

    A switch's table load (`lui at, hi; addu at, at, idx; lw rD, lo(at); jr rD`, or
    the pair landing on an address in `jtbls`) is named jtbl_<ADDR>, the name m2c
    looks for before a `jr`; decompile() supplies the table itself."""
    lines = asm.splitlines()
    parsed = []
    for l in lines:
        m = INSN_RE.match(l)
        parsed.append((m.group(2), [o.strip() for o in m.group(3).split(",")] if m and m.group(3) else []) if m else None)
    jt_lines = jr_loads(parsed, lines)

    # 1. calls
    for i, p in enumerate(parsed):
        if p and p[0] in ("jal", "j") and p[1]:
            m = re.fullmatch(r"func_([0-9a-f]+)", p[1][-1])
            if m:
                a = int(m.group(1), 16)
                lines[i] = lines[i].replace(p[1][-1], smap.func(a) or "func_%08X" % a)

    # 2. lui hi / lo pairs.  A lui opens a group in `live`; loads, stores and addiu
    # based on that register join it; any other write to the register closes it.
    # Labels and jump delay slots end the straight-line run: the group moves to
    # `stale`, and a later use through a stale register vetoes the rewrite (the
    # value may come from another path).
    groups, live, stale = [], {}, {}

    def kill(reg):
        live.pop(reg, None)
        stale.pop(reg, None)

    def use(reg, li, imm):
        if reg in live:
            g = live[reg]
            g["uses"].append((li, ((g["hi"] << 16) + imm) & 0xFFFFFFFF))
        elif reg in stale:
            stale[reg]["ok"] = False

    def end_run():
        stale.update(live)
        live.clear()

    after_jump = False
    for i, p in enumerate(parsed):
        if p is None:
            if lines[i].rstrip().endswith(":"):
                end_run()
            continue
        mnem, ops = p
        if mnem == "lui" and len(ops) == 2:
            kill(ops[0])
            g = dict(lui=i, hi=int(ops[1], 0) & 0xFFFF, uses=[], ok=True)
            groups.append(g)
            live[ops[0]] = g
        elif mnem in MEM_OPS and len(ops) == 2:
            m = re.fullmatch(r"(-?(?:0x)?[0-9a-fA-F]+)\((\$\w+)\)", ops[1])
            if m:
                use(m.group(2), i, int(m.group(1), 0))
            if mnem not in NO_DEST:
                kill(ops[0])
        elif mnem == "addiu" and len(ops) == 3:
            use(ops[1], i, int(ops[2], 0))
            kill(ops[0])
        elif (mnem == "addu" and len(ops) == 3 and ops[0] in live and ops[0] in ops[1:]
              and ops[1] != ops[2]):
            # lui at, hi; addu at, at, idx: a table indexed off the symbol.  Kept
            # only if every use turns out to be a jump-table load (checked below).
            live[ops[0]]["indexed"] = True
            other = ops[2] if ops[1] == ops[0] else ops[1]
            if other in stale:
                stale[other]["ok"] = False
            if other in live:
                live[other]["ok"] = False
        else:
            for o in ops[1:]:
                if o in stale:
                    stale[o]["ok"] = False      # value flows on through ALU ops: leave raw
                if o in live:
                    live[o]["ok"] = False
            if mnem not in NO_DEST and ops and ops[0].startswith("$"):
                kill(ops[0])
            if mnem in ("jal", "jalr"):
                kill("$ra")
        if after_jump:
            end_run()
            after_jump = False
        if mnem in ("j", "jr", "jal", "jalr", "b"):
            after_jump = True

    for g in groups:
        lui_i, uses = g["lui"], g["uses"]
        if not uses or not g["ok"]:
            continue
        names = []
        for li, addr in uses:
            jt = (li in jt_lines and g.get("indexed")) or addr in jtbls
            names.append("jtbl_%08X" % addr if jt else smap.lookup(addr))
        if any(n is None for n in names):
            continue
        if g.get("indexed") and not all(n.startswith("jtbl_") for n in names):
            continue
        bases = {n.split("+")[0] for n in names}
        if len(bases) != 1:
            continue
        lines[lui_i] = re.sub(r",\s*\S+$", ", %%hi(%s)" % names[0], lines[lui_i])
        for (li, addr), n in zip(uses, names):
            mnem, ops = parsed[li]
            if mnem == "addiu":
                lines[li] = re.sub(r",\s*\S+$", ", %%lo(%s)" % n, lines[li])
            else:
                lines[li] = re.sub(r",\s*-?(?:0x)?[0-9a-fA-F]+\((\$\w+)\)$", r", %%lo(%s)(\1)" % n, lines[li])
    return "\n".join(lines) + "\n"


JTBL_RE = re.compile(r"%lo\(jtbl_([0-9A-F]{8})\)")
JR_REG_RE = re.compile(r"^\s+jr\s+\$(?!ra\b)\w+\s*$", re.M)


class JumpTables:
    """The `jtbls` symbolise() takes for one function: its jt_addrs from
    functions.tsv, plus (when the function has a `jr` through a register) any
    address whose retail word points strictly inside the function.  The second
    catches a table the inventory missed whose base GCC hoisted out of a loop
    (lui/addiu s0 before the loop, addu idx, s0; lw; jr inside it)."""

    def __init__(self, known, asm, vaddr, length, read_word):
        self.known = set(known)
        self.scan = bool(JR_REG_RE.search(asm))
        self.vaddr, self.length, self.read_word = vaddr, length, read_word

    def __contains__(self, addr):
        if addr in self.known:
            return True
        if not self.scan:
            return False
        w = self.read_word(addr)
        return w is not None and not w & 3 and self.vaddr < w < self.vaddr + self.length


def add_jump_tables(asm, vaddr, length, read_word, known=()):
    """Give m2c the jump tables symbolise() named: label every case target in the
    function (.L<addr>, as objdump2m2c names branch targets) and append

        .section .rodata
        jtbl_<ADDR>:
        .word .L<case>  ...

    read_word(addr) -> the retail word or None.  A table runs until a word that is
    not an instruction address of this function, or the next table (any jtbl_ the
    asm reads, or an address in `known`, e.g. the function's jt_addrs)."""
    addrs = sorted({int(a, 16) for a in JTBL_RE.findall(asm)})
    if not addrs:
        return asm
    stops = set(addrs) | set(known)
    tables = []
    for a in addrs:
        words, p = [], a
        while len(words) < 1024:
            w = read_word(p)
            if w is None or w & 3 or not vaddr <= w < vaddr + length:
                break
            words.append(w)
            p += 4
            if p in stops:
                break
        if words:
            tables.append((a, words))
    targets = {w for _, ws in tables for w in ws}
    have = {int(h, 16) for h in re.findall(r"^\.L([0-9a-f]+):", asm, re.M)}
    out, k = [], 0
    for l in asm.splitlines():
        if INSN_RE.match(l):
            addr = vaddr + 4 * k
            if k < length // 4 and addr in targets and addr not in have:
                out.append(".L%x:" % addr)
                have.add(addr)
            k += 1
        out.append(l)
    if tables:
        out += ["", ".section .rodata"]
        for a, ws in tables:
            out.append("jtbl_%08X:" % a)
            out += [".word .L%x" % w for w in ws]
    return "\n".join(out) + "\n"


def function_row(vaddr, unit):
    """(size, [jt addrs]) of a function in tools/loop/functions.tsv, or None."""
    fu = "ovl" + unit if unit.isdigit() else unit
    if os.path.isfile(FUNCTIONS):
        with open(FUNCTIONS, encoding="utf-8") as fh:
            head = fh.readline().rstrip("\r\n").split("\t")
            jt = head.index("jt_addrs") if "jt_addrs" in head else None
            for line in fh:
                p = line.rstrip("\r\n").split("\t")
                if len(p) > 2 and p[0] == fu and p[1].startswith("0x") and int(p[1], 16) == vaddr:
                    jts = p[jt] if jt is not None and jt < len(p) else ""
                    return int(p[2]), [int(x, 16) for x in jts.split(",") if x.strip()]
    return None


def function_size(vaddr, unit):
    row = function_row(vaddr, unit)
    if row:
        return row[0]
    fu = "ovl" + unit if unit.isdigit() else unit
    raise CtxError("0x%08X (%s) is not in tools/loop/functions.tsv; pass LEN" % (vaddr, fu))


def unit_image(unit):
    sys.path.insert(0, os.path.join(ROOT, "tools", "loop"))
    import units  # noqa: E402
    u = units.resolve(unit)
    return u.image_path, u.base


def decompile(vaddr, length, unit, protos=(), raw=False, ctx_path=None, m2c_args=()):
    """m2c's C for one function; m2c_args are passed through (e.g. --valid-syntax)."""
    image, base = unit_image(unit)
    with open(image, "rb") as fh:
        fh.seek(vaddr - base)
        blob = fh.read(length)
    if len(blob) != length:
        raise CtxError("0x%08X+%d is past the end of %s" % (vaddr, length, image))
    tmp = tempfile.mkdtemp(prefix="ac1ctx_")
    try:
        sl = os.path.join(tmp, "slice.bin")
        with open(sl, "wb") as fh:
            fh.write(blob)
        dump = run([mips_tool("objdump"), "-D", "-b", "binary", "-m", "mips:isa32", "-EL",
                    "--adjust-vma=0x%x" % vaddr, sl])
        # objdump2m2c turns every `j` into a call; a `j` that lands inside this
        # function is GCC's jump to a shared epilogue, i.e. a plain branch
        def in_range_j(m):
            inside = vaddr <= int(m.group(3), 16) < vaddr + length
            return m.group(1) + ("b" if inside else "j") + m.group(2)
        dump = re.sub(r"(\t)j(\s+0x([0-9a-f]+))$", in_range_j, dump, flags=re.M)
        name = "func_%08X" % vaddr
        asm = subprocess.run([sys.executable, OBJDUMP2M2C, "%x" % vaddr, name], input=dump,
                             capture_output=True, text=True, errors="replace")
        if asm.returncode:
            raise CtxError("objdump2m2c failed:\n" + asm.stderr)
        sys.stderr.write(asm.stderr)
        text = asm.stdout
        cmd = m2c_cmd() + ["-t", "mips-gcc-c"] + list(m2c_args)
        if not raw:
            row = function_row(vaddr, unit_key(unit))
            jts = row[1] if row else []

            def read_word(a):
                if a < base:
                    return None
                fh.seek(a - base)
                b = fh.read(4)
                return int.from_bytes(b, "little") if len(b) == 4 else None
            with open(image, "rb") as fh:
                text = symbolise(text, SymbolMap(unit_key(unit)),
                                 JumpTables(jts, text, vaddr, length, read_word))
                text = add_jump_tables(text, vaddr, length, read_word, jts)
            ctx = ctx_path or os.path.join(tmp, "ctx.c")
            write_context(ctx, protos)
            cmd += ["--context", ctx, "--no-cache"]
        fs = os.path.join(tmp, "fn.s")
        with open(fs, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        return run(cmd + [fs])
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


# ----------------------------------------------------------------------------
# check
# ----------------------------------------------------------------------------
def check():
    problems = []
    rows = load_syms()
    want = render_symbols_h(rows)
    if not os.path.isfile(SYMS_H) or open(SYMS_H, encoding="utf-8").read() != want:
        problems.append("include/ac1/symbols.h is stale (run ctx.py gen)")
    for fn in sorted(os.listdir(AC1)):
        if not fn.endswith(".h"):
            continue
        src = open(os.path.join(AC1, fn), encoding="utf-8").read()
        code = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), src, flags=re.S)
        code = re.sub(r'"(?:\\.|[^"\\])*"', '""', code)
        for n, line in enumerate(code.splitlines(), 1):
            if "//" in line:
                problems.append("%s:%d: // comment (GCC 2.7.2 C has none)" % (fn, n))
    # GCC 2.7.2 dialect + every size assert, with a modern MIPS gcc in strict C89 mode
    tmp = tempfile.mkdtemp(prefix="ac1chk_")
    try:
        t = os.path.join(tmp, "t.c")
        with open(t, "w") as fh:
            fh.write('#include "ac1/ac1.h"\n')
        r = subprocess.run([mips_tool("gcc"), "-std=c89", "-pedantic-errors", "-Wall", "-fsyntax-only",
                            "-nostdinc", "-I", INC, t], capture_output=True, text=True, errors="replace")
        if r.returncode:
            problems.append("headers do not compile as C89 (or a size assert fails):\n" + r.stderr)
        if not problems:
            ctx = write_context(os.path.join(tmp, "ctx.c"))
            dummy = os.path.join(tmp, "t.s")
            with open(dummy, "w") as fh:
                fh.write("glabel ctx_check\n    jr $ra\n    nop\n")
            r = subprocess.run(m2c_cmd() + ["--context", ctx, "--no-cache", "--dump-typemap", dummy],
                               capture_output=True, text=True, errors="replace")
            if r.returncode:
                problems.append("m2c rejects the context:\n" + r.stdout + r.stderr)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    names = {r["name"] for r in rows}
    print("%d symbols, %d struct sizes asserted" % (len(names), len(struct_sizes()) - len(PRIM)))
    for p in problems:
        print("FAIL " + p)
    print("OK" if not problems else "%d problem(s)" % len(problems))
    return 0 if not problems else 1


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("gen", help="regenerate include/ac1/symbols.h")
    sub.add_parser("check", help="validate headers, symbols.h and the m2c context")
    c = sub.add_parser("context", help="write the m2c context file")
    c.add_argument("-o", "--out", default=DEFAULT_CTX)
    c.add_argument("--proto", action="append", default=[])
    m = sub.add_parser("m2c", help="decompile one function with the context")
    m.add_argument("vaddr")
    m.add_argument("length", nargs="?")
    m.add_argument("--unit", default="201", help="exe, 201..204 or fdatNNN (default 201)")
    m.add_argument("--proto", action="append", default=[])
    m.add_argument("--raw", action="store_true", help="no context, no symbol rewrite")
    m.add_argument("--keep-context", metavar="FILE", help="also write the context used here")
    a = ap.parse_args(argv)
    try:
        if a.cmd == "gen":
            text = render_symbols_h(load_syms())
            with open(SYMS_H, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(text)
            print("wrote " + os.path.relpath(SYMS_H, ROOT))
            return 0
        if a.cmd == "check":
            return check()
        if a.cmd == "context":
            print("wrote " + write_context(a.out, a.proto))
            return 0
        if a.cmd == "m2c":
            v = int(a.vaddr, 16)
            unit = unit_key(a.unit)
            n = int(a.length, 0) if a.length else function_size(v, unit)
            sys.stdout.write(decompile(v, n, unit, a.proto, a.raw, a.keep_context))
            return 0
    except CtxError as e:
        print("ERROR: %s" % e, file=sys.stderr)
        return 2
    return 3


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
