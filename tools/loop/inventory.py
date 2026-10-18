#!/usr/bin/env python3
"""inventory.py - list every function in the AC1 executable and its code overlays.

Writes tools/loop/functions.tsv, one row per function, for the matching loop
(rank.py picks candidates from it, ledger.py records outcomes against it).

Columns
    unit          exe | ovl201..ovl204 | fdatNNN (mission overlays, --missions)
    vaddr         0x-prefixed RAM address of the first instruction
    size          bytes, including the delay slot after the final jr ra
    name          ac1_markup.tsv name for exe functions, func_<ADDR> otherwise
                  (overlay-range markup names are ambiguous between overlays)
    is_psyq       1 inside the PsyQ library block of the exe (excluded by rank.py)
    instructions  size / 4
    branches      conditional branches + j (not jal/jalr/jr)
    calls         jal count
    jump_tables   jr <reg> through a switch table (lui/addu/lw ... jr pattern)
    gte_ops       COP2 instructions (c2/mtc2/mfc2/ctc2/cfc2) + lwc2/swc2
    stack_size    frame bytes from the prologue addiu sp,sp,-N (0 = leaf, no frame)
    jt_addrs      comma-separated RAM addresses of the jump tables (for
                  match_fn.py --rodata), empty if none
    evidence      why this address is a function start: text (unit start),
                  ret (follows jr ra + delay slot), jal (jal target),
                  pro (addiu sp,sp,-N prologue), mk (ac1_markup.tsv row),
                  ptr (code pointer stored in rodata/data)

Method
    Each unit is disassembled once with
        <MIPS_PREFIX>objdump -D -b binary -m mips:3000 -EL --adjust-vma=<base>
    The .text range comes from the known layout (exe, ovl201), from
    overlays/CODE_UNITS.tsv when present (--code-units), or is detected:
    first stack-frame prologue to the last jr ra that ends a run of decodable
    instructions.  Inside .text every branch / j / jump-table target is a span
    (source -> target); a candidate boundary that any span crosses is inside a
    function.  Candidates are: the instruction after a `jr ra` + delay slot,
    jal targets, prologues, markup rows (exe only), and code pointers found in
    rodata/data.  A candidate with zero crossing spans starts a new function.
    For the exe the detected starts are compared with the F rows of
    ac1_markup.tsv and the agreement is printed before the markup rows are
    merged in.

Environment
    AC1_BIN       disc image (.bin); the PS-X EXE is carved from sectors 39-121
    MIPS_PREFIX   cross toolchain prefix, e.g. mipsel-none-elf- (default: auto)
    AC1_DISC_MAP  disc_map directory if not <repo>/disc_map (gitignored binaries
                  live in the main checkout, which is also searched automatically)

Example
    MIPS_PREFIX=mipsel-none-elf- python tools/loop/inventory.py
    python tools/loop/inventory.py --units ovl202 --out /tmp/ovl202.tsv
    python tools/loop/inventory.py --missions            # + 32 mission overlays
"""
import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
DEFAULT_OUT = os.path.join(HERE, "functions.tsv")
MARKUP = os.path.join(REPO, "ac1_markup.tsv")

COLUMNS = ["unit", "vaddr", "size", "name", "is_psyq", "instructions", "branches",
           "calls", "jump_tables", "gte_ops", "stack_size", "jt_addrs", "evidence"]

# ---------------------------------------------------------------------------
# layout facts (SLUS-01323 == SCUS-94182 code, verified 2026-10-18)
# ---------------------------------------------------------------------------
EXE_SECTORS = (39, 121)        # raw 2352-byte sectors holding the PS-X EXE
EXE_TADDR = 0x80011000         # t_addr; file offset 0 (the 0x800 header) = 0x80010800
EXE_FILE_VA = EXE_TADDR - 0x800
EXE_TEXT = (0x80011E64, 0x800315B4)   # __main .. start of .data (file 0x20DB4)

OVERLAY_BASE = 0x8004ADA0      # ovl201..204 all load here
OVL201_TEXT = (0x8004DCC0, 0x800B6DEC)  # rodata 0..0x2F20, text 0x2F20..0x6C04C

# PsyQ library objects linked into the exe (never game C): crt0, the lib block,
# and the two card-driver stubs that sit right before .data.
PSYQ_RANGES = (
    (0x80011E64, 0x80011F28, "crt0 (__main/start/stup)"),
    # 0x80030368..0x800315B4 is still library code (AddCDROMDevice-style
    # EnterCriticalSection/AddDrv/ExitCriticalSection wrappers, AddDrv, DelDrv)
    (0x80016EA4, 0x800315B4, "PsyQ 3.7 libraries"),
)

# mission overlays: FDAT_T entry number -> load address (disc_map/extracted/FDAT_T/entries)
MISSION_ENTRIES = {}
for _n in list(range(0, 99, 2)) + [120, 140]:
    MISSION_ENTRIES[_n] = 0x801C4B40
for _n in (160, 162, 164, 166, 168):
    MISSION_ENTRIES[_n] = 0x801C8460
for _n in (180, 182, 184, 186):
    MISSION_ENTRIES[_n] = 0x801B80C8

# a prologue this many words after another start candidate belongs to it
PROLOGUE_SLACK = 4

PROGRAM_OVERLAYS = {201: "front_end", 202: "mission", 203: "mode1", 204: "mode2"}


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------
def die(msg):
    sys.exit("inventory.py: " + msg)


def warn(msg):
    print("inventory.py: " + msg, file=sys.stderr)


def s16(v):
    v &= 0xFFFF
    return v - 0x10000 if v & 0x8000 else v


def find_objdump():
    prefix = os.environ.get("MIPS_PREFIX")
    cands = [prefix] if prefix else ["mipsel-none-elf-", "mipsel-linux-gnu-", "mips-linux-gnu-"]
    for p in cands:
        exe = shutil.which(p + "objdump")
        if exe:
            return exe
    die("no <prefix>objdump on PATH (set MIPS_PREFIX, e.g. mipsel-none-elf-)")


def disc_map_dirs():
    """Candidate disc_map directories: env, this checkout, the main git checkout."""
    out = []
    env = os.environ.get("AC1_DISC_MAP")
    if env:
        out.append(env)
    out.append(os.path.join(REPO, "disc_map"))
    try:
        common = subprocess.run(["git", "rev-parse", "--git-common-dir"], cwd=REPO,
                                capture_output=True, text=True, check=True).stdout.strip()
        if common:
            if not os.path.isabs(common):
                common = os.path.join(REPO, common)
            out.append(os.path.join(os.path.dirname(os.path.abspath(common)), "disc_map"))
    except (subprocess.CalledProcessError, OSError):
        pass
    seen, uniq = set(), []
    for d in out:
        d = os.path.normpath(d)
        if d not in seen:
            seen.add(d)
            uniq.append(d)
    return uniq


def find_in_disc_map(rel):
    for d in disc_map_dirs():
        p = os.path.join(d, rel)
        if os.path.exists(p):
            return p
    return None


def carve_exe():
    disc = os.environ.get("AC1_BIN")
    if not disc or not os.path.isfile(disc):
        die("AC1_BIN is not set or does not point at the disc image")
    data = bytearray()
    with open(disc, "rb") as fh:
        for s in range(EXE_SECTORS[0], EXE_SECTORS[1] + 1):
            fh.seek(s * 2352 + 24)
            data += fh.read(2048)
    if data[:8] != b"PS-X EXE":
        die("sectors %d-%d of AC1_BIN are not a PS-X EXE" % EXE_SECTORS)
    t_addr, t_size = struct.unpack_from("<II", data, 0x18)
    if t_addr != EXE_TADDR:
        warn("exe t_addr is 0x%08X, expected 0x%08X" % (t_addr, EXE_TADDR))
    return bytes(data[:0x800 + t_size])


def overlay_image(entry):
    """(bytes, base) for program overlay 201..204 via disc_map/overlays/overlays.json."""
    j = find_in_disc_map(os.path.join("overlays", "overlays.json"))
    if not j:
        die("disc_map/overlays/overlays.json not found (run tools/extract/extract_overlay.py "
            "or set AC1_DISC_MAP)")
    with open(j) as fh:
        meta = json.load(fh)
    for o in meta.get("overlays", []):
        if o.get("entry") == entry:
            p = os.path.join(os.path.dirname(j), os.path.basename(o["bin"].replace("\\", "/")))
            if not os.path.isfile(p):
                die("overlay image %s missing" % p)
            with open(p, "rb") as fh:
                return fh.read(), o.get("load_base", OVERLAY_BASE)
    die("overlay %d not in overlays.json" % entry)


def mission_image(entry):
    p = find_in_disc_map(os.path.join("extracted", "FDAT_T", "entries", "%03d.bin" % entry))
    if not p:
        return None
    with open(p, "rb") as fh:
        return fh.read()


# ---------------------------------------------------------------------------
# disassembly
# ---------------------------------------------------------------------------
LINE_RE = re.compile(r"^\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(\S+)\s*(.*)$")
ELLIPSIS_RE = re.compile(r"^\s*\.\.\.\s*$")


class Listing:
    """Word-indexed disassembly of one unit image (whole file)."""

    def __init__(self, image, base, objdump, tmpdir):
        self.image = image
        self.base = base
        self.n = len(image) // 4
        self.words = list(struct.unpack_from("<%dI" % self.n, image, 0))
        self.valid = [False] * self.n          # objdump decoded it as an instruction
        path = os.path.join(tmpdir, "unit_%08x.bin" % base)
        with open(path, "wb") as fh:
            fh.write(image)
        cmd = [objdump, "-D", "-b", "binary", "-m", "mips:3000", "-EL",
               "--adjust-vma=0x%X" % base, path]
        res = subprocess.run(cmd, capture_output=True, text=True, errors="replace")
        if res.returncode != 0:
            die("objdump failed: %s" % res.stderr.strip())
        for line in res.stdout.splitlines():
            m = LINE_RE.match(line)
            if not m:
                continue  # headers, "..." (zero runs = nops, valid, left as is)
            addr = int(m.group(1), 16)
            idx = (addr - base) // 4
            if 0 <= idx < self.n:
                self.valid[idx] = not m.group(3).startswith(".")
        # objdump collapses zero runs into "..."; zeros are nops
        for i in range(self.n):
            if self.words[i] == 0:
                self.valid[i] = True
        os.unlink(path)

    def idx(self, va):
        return (va - self.base) // 4

    def va(self, idx):
        return self.base + idx * 4

    def word_at(self, va):
        i = self.idx(va)
        return self.words[i] if 0 <= i < self.n else None


# instruction predicates on raw words -------------------------------------
def op(w):
    return w >> 26


def rs(w):
    return (w >> 21) & 31


def rt(w):
    return (w >> 16) & 31


def rd(w):
    return (w >> 11) & 31


def funct(w):
    return w & 63


def is_jal(w):
    return op(w) == 3


def is_j(w):
    return op(w) == 2


def jump_target(w, va):
    return ((va + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)


def is_branch(w):
    o = op(w)
    return o in (4, 5, 6, 7) or (o == 1 and rt(w) in (0, 1, 16, 17))


def branch_target(w, va):
    return va + 4 + (s16(w) << 2)


def is_jr_ra(w):
    return op(w) == 0 and funct(w) == 8 and rs(w) == 31


def is_jr_reg(w):
    return op(w) == 0 and funct(w) == 8 and rs(w) != 31


def is_jalr(w):
    return op(w) == 0 and funct(w) == 9


def prologue_frame(w):
    """addiu sp,sp,-N -> N, else 0."""
    if op(w) == 9 and rs(w) == 29 and rt(w) == 29:
        imm = s16(w)
        if imm < 0:
            return -imm
    return 0


def is_sw_ra_sp(w):
    return op(w) == 43 and rs(w) == 29 and rt(w) == 31


def is_gte(w):
    return op(w) in (0x12, 0x32, 0x3A)


def is_lui(w):
    return op(w) == 15


def is_lw(w):
    return op(w) == 35


def is_sltiu(w):
    return op(w) == 11


# ---------------------------------------------------------------------------
# text range detection
# ---------------------------------------------------------------------------
def detect_text(lst):
    """Return (start_va, end_va) of the code region, or None.

    start: first `addiu sp,sp,-N` (8 <= N <= 4096, multiple of 8) followed within
           12 words by `sw ra,M(sp)`, moved back to the earliest jal target /
           stored code pointer inside the decodable run that precedes it.
    end:   after the last `jr ra` + delay slot that ends a run of >= 24 decodable
           words.
    """
    n = lst.n
    start = None
    for i in range(n - 12):
        fr = prologue_frame(lst.words[i])
        if 8 <= fr <= 4096 and fr % 8 == 0:
            if any(is_sw_ra_sp(lst.words[j]) for j in range(i + 1, i + 13)):
                start = i
                break
    if start is None:
        return None
    # Leaf functions (no frame) and loads hoisted above the prologue may precede
    # the first framed function.  Walk back over decodable words and take the
    # earliest address in that run that is a jal target or a stored code pointer.
    run_lo = start
    while run_lo > 0 and lst.valid[run_lo - 1]:
        run_lo -= 1
    targets = set()
    for k in range(n):
        w = lst.words[k]
        if is_jal(w):
            t = lst.idx(jump_target(w, lst.va(k)))
        elif w & 3 == 0:
            t = lst.idx(w)
        else:
            continue
        if run_lo <= t <= start and not (run_lo <= k <= start and not is_jal(w)):
            targets.add(t)
    if targets:
        start = min(targets)
    else:
        # nothing points at the first function: accept the words GCC hoists
        # above `addiu sp` (lui, non-byte loads, varargs `sw aN,M(sp)`), none of
        # which can be mistaken for a 0x80xxxxxx pointer left over from rodata
        for _ in range(PROLOGUE_SLACK):
            w = lst.words[start - 1] if start > 0 else 0
            if w and lst.valid[start - 1] and (is_lui(w) or 33 <= op(w) <= 38
                                               or (40 <= op(w) <= 43 and rs(w) == 29)):
                start -= 1
            else:
                break
    end = None
    for i in range(n - 2, start, -1):
        if is_jr_ra(lst.words[i]) and lst.valid[i + 1]:
            lo = max(start, i - 24)
            if all(lst.valid[j] for j in range(lo, i)):
                end = i + 2
                break
    if end is None:
        return None
    return lst.va(start), lst.va(end)


# ---------------------------------------------------------------------------
# function splitting
# ---------------------------------------------------------------------------
def find_jump_table(lst, i, text_lo_i, text_hi_i):
    """For `jr rX` at word index i, locate the switch table it dispatches through.

    Returns (table_va, [target_va...]) or None.  Pattern (GCC 2.7.2 -O2):
        sltiu  t,idx,N        (optional, gives the entry count)
        ...
        lui    at,%hi(tab)
        addu   at,at,idx*4
        lw     rX,%lo(tab)(at)
        nop
        jr     rX
    """
    w = lst.words[i]
    reg = rs(w)
    lo_i = max(text_lo_i, i - 12)
    lw_i = None
    for j in range(i - 1, lo_i - 1, -1):
        wj = lst.words[j]
        if is_lw(wj) and rt(wj) == reg:
            lw_i = j
            break
        if (op(wj) == 0 and rd(wj) == reg and wj != 0) or (op(wj) in (8, 9, 12, 13, 14, 15) and rt(wj) == reg):
            return None  # reg written by something that is not a table load
    if lw_i is None:
        return None
    base_reg = rs(lst.words[lw_i])
    lo = s16(lst.words[lw_i])
    hi = None
    for j in range(lw_i - 1, max(text_lo_i, lw_i - 8) - 1, -1):
        wj = lst.words[j]
        if is_lui(wj) and rt(wj) == base_reg:
            hi = wj & 0xFFFF
            break
    if hi is None:
        return None
    table = ((hi << 16) + lo) & 0xFFFFFFFF
    count = None
    for j in range(i - 1, max(text_lo_i, i - 14) - 1, -1):
        wj = lst.words[j]
        if is_sltiu(wj):
            count = wj & 0xFFFF
            break
    ti = lst.idx(table)
    if not (0 <= ti < lst.n):
        return table, []
    limit = count if count else 32
    jr_va = lst.va(i)
    targets = []
    for k in range(limit):
        if ti + k >= lst.n:
            break
        t = lst.words[ti + k]
        tidx = lst.idx(t)
        if t & 3 or not (text_lo_i <= tidx < text_hi_i):
            break
        if count is None and abs(t - jr_va) > 0x4000:
            break
        targets.append(t)
    return table, targets


def jr_to_immediate(lst, i, lo_i):
    """`jr rX` where rX was just loaded with a constant (li/lui): a tail jump
    into the BIOS (AddDrv-style thunks), i.e. the end of a function."""
    reg = rs(lst.words[i])
    for j in range(i - 1, max(lo_i, i - 4) - 1, -1):
        wj = lst.words[j]
        if rt(wj) == reg and ((op(wj) in (9, 13) and rs(wj) == 0) or is_lui(wj)):
            return True
    return False


def code_pointers_outside_text(lst, lo_i, hi_i, exclude):
    """Word-aligned values stored outside .text that point into .text."""
    out = set()
    for k in range(lst.n):
        if lo_i <= k < hi_i or k in exclude:
            continue
        t = lst.words[k]
        if t & 3:
            continue
        ti = lst.idx(t)
        if lo_i <= ti < hi_i:
            out.add(t)
    return out


def split_functions(lst, text, markup_starts=None, use_markup=False):
    """Return (functions, heuristic_starts).

    functions: list of dicts with vaddr/size/... for the .text range.
    heuristic_starts: set of start VAs found *without* markup rows (for the
    agreement report).
    """
    lo_i, hi_i = lst.idx(text[0]), lst.idx(text[1])
    words = lst.words

    # 1. spans and candidates ------------------------------------------------
    cross = [0] * (hi_i - lo_i + 2)   # cross[k-lo_i] = spans crossing boundary before word k
    cand = {}                          # start idx -> set(evidence)

    def add_span(a, b):
        if a == b:
            return
        lo, hi = min(a, b), max(a, b)
        # boundary at index k is crossed iff lo < k <= hi
        cross[lo + 1 - lo_i] += 1
        cross[hi + 1 - lo_i] -= 1

    def add_cand(idx, ev):
        if lo_i < idx < hi_i:
            cand.setdefault(idx, set()).add(ev)

    prologues = []
    jt_by_jr = {}     # jr idx -> table va
    jt_words = set()  # indices of jump-table entry words (not function pointers)
    cand[lo_i] = {"text"}
    for i in range(lo_i, hi_i):
        w = words[i]
        va = lst.va(i)
        if is_branch(w) or is_j(w):
            t = branch_target(w, va) if is_branch(w) else jump_target(w, va)
            ti = lst.idx(t)
            if lo_i <= ti < hi_i:
                add_span(i, ti)
        elif is_jal(w):
            ti = lst.idx(jump_target(w, va))
            add_cand(ti, "jal")
        elif is_jr_reg(w):
            jt = find_jump_table(lst, i, lo_i, hi_i)
            if jt:
                table, targets = jt
                jt_by_jr[i] = table
                tbi = lst.idx(table)
                for k, t in enumerate(targets):
                    add_span(i, lst.idx(t))
                    jt_words.add(tbi + k)
            elif jr_to_immediate(lst, i, lo_i) and i + 2 < hi_i:
                add_cand(i + 2, "ret")   # BIOS thunk: li t2,0xB0 ; jr t2 ; li t1,N
        elif is_jr_ra(w) and i + 2 < hi_i:
            add_cand(i + 2, "ret")
        if prologue_frame(w):
            prologues.append(i)
    # GCC 2.7.2 -O2 hoists up to a few loads above `addiu sp,sp,-N`, so a
    # prologue shortly after another candidate confirms that candidate instead
    # of starting a function of its own.
    for i in prologues:
        for back in range(0, PROLOGUE_SLACK + 1):
            if i - back in cand:
                cand[i - back].add("pro")
                break
        else:
            add_cand(i, "pro")
    for t in code_pointers_outside_text(lst, lo_i, hi_i, jt_words):
        add_cand(lst.idx(t), "ptr")

    # prefix-sum the crossing counts
    acc = 0
    crossing = [0] * len(cross)
    for k in range(len(cross)):
        acc += cross[k]
        crossing[k] = acc

    def is_start(idx):
        return crossing[idx - lo_i] == 0

    heuristic = {lst.va(lo_i)} | {lst.va(i) for i in cand if is_start(i)}

    if use_markup and markup_starts:
        for va in markup_starts:
            add_cand(lst.idx(va), "mk")

    starts = sorted(i for i in cand if is_start(i))

    # 2. merge nop-only blocks (alignment padding) into the previous function --
    bounds = []
    for k, s in enumerate(starts):
        e = starts[k + 1] if k + 1 < len(starts) else hi_i
        if bounds and all(words[j] == 0 for j in range(s, e)):
            bounds[-1][1] = e
            continue
        bounds.append([s, e])

    # 3. per-function stats ---------------------------------------------------
    funcs = []
    for s, e in bounds:
        branches = calls = gte = 0
        jts = []
        for i in range(s, e):
            w = words[i]
            if is_branch(w) or is_j(w):
                branches += 1
            elif is_jal(w):
                calls += 1
            elif is_gte(w):
                gte += 1
            if i in jt_by_jr:
                jts.append(jt_by_jr[i])
        stack = 0
        for i in range(s, min(e, s + PROLOGUE_SLACK + 4)):
            fr = prologue_frame(words[i])
            if fr:
                stack = fr
                break
        funcs.append({
            "vaddr": lst.va(s),
            "size": (e - s) * 4,
            "instructions": e - s,
            "branches": branches,
            "calls": calls,
            "jump_tables": len(jts),
            "gte_ops": gte,
            "stack_size": stack,
            "jt_addrs": ",".join("0x%08X" % t for t in jts),
            "evidence": "+".join(sorted(cand.get(s, {"text"}))),
        })
    return funcs, heuristic


# ---------------------------------------------------------------------------
# markup
# ---------------------------------------------------------------------------
def load_markup(path):
    """{va: name} for F rows of ac1_markup.tsv."""
    names = {}
    if not os.path.isfile(path):
        warn("markup %s not found; exe functions get func_<ADDR> names" % path)
        return names
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            if len(parts) >= 3 and parts[0] == "F":
                try:
                    va = int(parts[1], 16)
                except ValueError:
                    continue
                names[va] = parts[2]
    return names


def load_code_units(path):
    """unit/entry -> (text_va, text_end_va) from overlays/CODE_UNITS.tsv."""
    out = {}
    if not path or not os.path.isfile(path):
        return out
    with open(path) as fh:
        header = fh.readline().rstrip("\n").split("\t")
        col = {h: k for k, h in enumerate(header)}
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            if len(parts) < len(header):
                continue
            try:
                rng = (int(parts[col["text_va"]], 16), int(parts[col["text_end_va"]], 16))
            except (KeyError, ValueError):
                continue
            kind = parts[col.get("kind", 2)]
            entry = parts[col.get("entry", 1)]
            if kind == "exe":
                out["exe"] = rng
            elif entry.isdigit():
                n = int(entry)
                out["ovl%d" % n if n in PROGRAM_OVERLAYS else "fdat%03d" % n] = rng
    return out


def is_psyq(unit, va):
    if unit != "exe":
        return 0
    return int(any(lo <= va < hi for lo, hi, _ in PSYQ_RANGES))


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------
def unit_list(args):
    units = []
    if args.units:
        for u in args.units.split(","):
            u = u.strip()
            if u.isdigit():
                n = int(u)
                u = "ovl%d" % n if n in PROGRAM_OVERLAYS else "fdat%03d" % n
            units.append(u)
    else:
        units = ["exe"] + ["ovl%d" % n for n in sorted(PROGRAM_OVERLAYS)]
        if args.missions:
            units += ["fdat%03d" % n for n in sorted(MISSION_ENTRIES)]
    return units


def load_unit(unit):
    """(image bytes, base va, label) for a unit name."""
    if unit == "exe":
        return carve_exe(), EXE_FILE_VA, "SLUS-01323 exe (carved from AC1_BIN)"
    m = re.fullmatch(r"ovl(\d+)", unit)
    if m:
        n = int(m.group(1))
        img, base = overlay_image(n)
        return img, base, "overlay %d (%s)" % (n, PROGRAM_OVERLAYS.get(n, "?"))
    m = re.fullmatch(r"fdat(\d+)", unit)
    if m:
        n = int(m.group(1))
        if n not in MISSION_ENTRIES:
            die("no known load address for FDAT entry %d" % n)
        img = mission_image(n)
        if img is None:
            return None
        return img, MISSION_ENTRIES[n], "mission overlay FDAT entry %d" % n
    die("unknown unit %r (exe, ovl201..ovl204, fdatNNN)" % unit)


def parse_text_overrides(items):
    out = {}
    for it in items or []:
        m = re.fullmatch(r"([^=]+)=(0x[0-9a-fA-F]+):(0x[0-9a-fA-F]+)", it)
        if not m:
            die("--text expects UNIT=0xSTART:0xEND, got %r" % it)
        out[m.group(1)] = (int(m.group(2), 16), int(m.group(3), 16))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog=__doc__.split("Columns", 1)[1] if "Columns" in __doc__ else None)
    ap.add_argument("--out", default=DEFAULT_OUT, help="output TSV (default tools/loop/functions.tsv)")
    ap.add_argument("--units", help="comma list: exe,ovl201,...,fdat000 (default exe + ovl201..204)")
    ap.add_argument("--missions", action="store_true", help="also inventory the mission overlays")
    ap.add_argument("--markup", default=MARKUP, help="ac1_markup.tsv (names + exe cross-check)")
    ap.add_argument("--no-markup-starts", action="store_true",
                    help="do not add markup rows as exe function starts (pure heuristic)")
    ap.add_argument("--code-units", default=os.path.join(REPO, "overlays", "CODE_UNITS.tsv"),
                    help="CODE_UNITS.tsv with text_va/text_end_va per unit (used when present)")
    ap.add_argument("--text", action="append", metavar="UNIT=0xSTART:0xEND",
                    help="override the .text range of a unit (repeatable)")
    ap.add_argument("--tmp", default=None, help="scratch dir for objdump input (default: system temp)")
    args = ap.parse_args()

    objdump = find_objdump()
    import tempfile
    tmpdir = args.tmp or tempfile.mkdtemp(prefix="ac1_inv_")
    os.makedirs(tmpdir, exist_ok=True)

    markup = load_markup(args.markup)
    known_text = {"exe": EXE_TEXT, "ovl201": OVL201_TEXT}
    cu = load_code_units(args.code_units)
    if cu:
        print("text ranges from %s: %d units" % (args.code_units, len(cu)))
    known_text.update(cu)
    known_text.update(parse_text_overrides(args.text))

    rows = []
    for unit in unit_list(args):
        loaded = load_unit(unit)
        if loaded is None:
            warn("%s: image not found, skipped" % unit)
            continue
        image, base, label = loaded
        lst = Listing(image, base, objdump, tmpdir)
        src = "layout"
        text = known_text.get(unit)
        if text is None:
            text = detect_text(lst)
            src = "detected"
            if text is None:
                warn("%s: no code region detected, skipped" % unit)
                continue
        if not (base <= text[0] < text[1] <= base + len(image)):
            die("%s: text range 0x%08X..0x%08X outside the image" % (unit, text[0], text[1]))

        mk_in = sorted(va for va in markup if text[0] <= va < text[1]) if unit == "exe" else []
        funcs, heur = split_functions(lst, text, mk_in,
                                      use_markup=(unit == "exe" and not args.no_markup_starts))

        print("%-8s %-40s text 0x%08X..0x%08X (%s, %d KB)  functions: %d"
              % (unit, label, text[0], text[1], src, (text[1] - text[0]) // 1024, len(funcs)))
        if unit == "exe" and mk_in:
            mk = set(mk_in)
            both = mk & heur
            print("  markup cross-check (heuristic starts vs %d markup F rows in .text):" % len(mk))
            print("    markup rows found by heuristic : %d/%d = %.1f%%"
                  % (len(both), len(mk), 100.0 * len(both) / len(mk)))
            print("    heuristic starts in markup     : %d/%d = %.1f%%"
                  % (len(both), len(heur), 100.0 * len(both) / max(1, len(heur))))
            game_mk = {v for v in mk if not is_psyq("exe", v)}
            game_h = {v for v in heur if not is_psyq("exe", v)}
            gb = game_mk & game_h
            print("    game code outside the PsyQ block: markup has %d row(s), heuristic found %d "
                  "function(s), %d in common" % (len(game_mk), len(game_h), len(gb)))
            print("    (lib-block markup rows include Ghidra's object-internal labels, which are "
                  "not function starts by flow)")
            miss = sorted(mk - heur)
            extra = sorted(heur - mk)
            if miss:
                print("    markup-only (first 12): " + " ".join("%08X" % v for v in miss[:12]))
            if extra:
                print("    heuristic-only (first 12): " + " ".join("%08X" % v for v in extra[:12]))
            if not args.no_markup_starts:
                print("    functions.tsv uses heuristic + markup starts (--no-markup-starts for heuristic only)")

        ev = {}
        for f in funcs:
            for e in f["evidence"].split("+"):
                ev[e] = ev.get(e, 0) + 1
        print("  evidence: " + ", ".join("%s=%d" % kv for kv in sorted(ev.items())))

        for f in funcs:
            va = f["vaddr"]
            name = markup.get(va) if unit == "exe" else None
            row = {
                "unit": unit,
                "vaddr": "0x%08X" % va,
                "size": f["size"],
                "name": name or "func_%08X" % va,
                "is_psyq": is_psyq(unit, va),
                "instructions": f["instructions"],
                "branches": f["branches"],
                "calls": f["calls"],
                "jump_tables": f["jump_tables"],
                "gte_ops": f["gte_ops"],
                "stack_size": f["stack_size"],
                "jt_addrs": f["jt_addrs"],
                "evidence": f["evidence"],
            }
            rows.append(row)

    with open(args.out, "w", newline="") as fh:
        fh.write("\t".join(COLUMNS) + "\n")
        for r in rows:
            fh.write("\t".join(str(r[c]) for c in COLUMNS) + "\n")
    psyq = sum(r["is_psyq"] for r in rows)
    print("wrote %s: %d functions (%d PsyQ library, %d candidates)"
          % (os.path.relpath(args.out, os.getcwd()), len(rows), psyq, len(rows) - psyq))
    if not args.tmp:
        shutil.rmtree(tmpdir, ignore_errors=True)


if __name__ == "__main__":
    main()
