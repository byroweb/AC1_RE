#!/usr/bin/env python3
"""twins.py - one matched function counts for every clone of it.

    twins.py group  [--scope missions|programs|all] [--functions F] [--out FILE]
    twins.py replay [--scope missions|programs|all] [--manifest re/match/manifest.tsv]
                    [--twins F] [--twins-programs F] [--report FILE] [--dry-run] [-j N]
    drive.py --replay-twins [same options as replay]   (forwards here)

Two scopes, grouped separately (a body is never credited across them):

missions  the 58 mission overlays (FDAT_T entries, units fdatNNN) link a common
          object library at per-entry addresses, so most functions occur in
          many entries with the same instructions and only their absolute
          addresses changed.  -> tools/loop/twins.tsv, groups gNNNN.
programs  the exe's game code (is_psyq 0) and the program overlays ovl201..204,
          which share the load address 0x8004ADA0; ovl202/203/204 (mission,
          mode 1, mode 2) carry much of the same code at shifted addresses.
          Keyed by (unit, vaddr) like everything else.
          -> tools/loop/twins_programs.tsv, groups pNNNN.  A program twin is
          compiled with its own unit's oracle flags (none for exe, --ovl 20N),
          and every name of the row's symbol tables (its --syms files, else the
          implicit ac1_markup.tsv) that sits at or up to 0xFFF below a
          relocation target whose address moved is passed as an explicit
          --defsym (which wins over the tables in match_fn.py) at the moved
          address.  A wrong guess can only cost a MISMATCH, never a MATCH.

group:  every row of the scope in functions.tsv (inventory.py --missions) is read from
        its image and *masked*: the 26-bit target of `jal`/`j`, the immediate of
        a `lui` that loads a RAM high half (0x8000..0x8020), and the low half
        (`addiu`/`ori` or a load/store offset) of every instruction whose base
        register still holds such a `lui` - i.e. exactly the words that carry a
        HI16/LO16/26 relocation.  Everything else is kept, so `addiu sp,sp,-N`,
        struct offsets and constants still distinguish functions.  Rows with the
        same masked body form a group; the representative is the member with
        the lowest (entry, vaddr).  twins.tsv gets one line per member with the
        relocation targets that differ from the representative's
        (`REP>MEMBER(+delta)`), the count that are equal, and a slot list.

replay: for every original manifest row of the scope (its source is in its
        own unit's directory and named after its address), every other member
        of its group is compiled from the *same*
        C file through tools/match/match_fn.py with the member's image/base,
        size, and --defsym / --rodata values derived from the member's own
        retail bytes: each `--defsym NAME=A` of the matched row is moved by
        the delta of the relocation slot that targets A (or the nearest slot
        at A+off, off < 0x1000); a symbol that no slot references is kept
        unchanged when it lies outside the matched unit's image and is an
        error otherwise.  A MATCH (exit 0, final line MATCH) adds a manifest
        row (source = the shared C file, extra = --base + derived symbols);
        anything else is listed in the report with the oracle's final line.
        The report (re/match/twin_replay.tsv) lists every attempted twin.
        Each new MATCH is also recorded in the ledger ($AC1_LEDGER or
        tools/loop/ledger.tsv) as `match` with attempts 0 (= unknown, which
        refit.py skips, so clones do not flatten the calibration sample) and
        reason `twin of <unit> <vaddr> (<source>)`, so rank.py and
        `drive.py next` stop offering it; --no-ledger skips that.
        --scope (default all) limits the run to one scope; report rows of the
        other scope are kept as they are.
"""
import argparse
import bisect
import concurrent.futures as cf
import os
import re
import shlex
import subprocess
import sys
from collections import Counter, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import ledger  # noqa: E402
import units  # noqa: E402
import merge_manifest  # noqa: E402

FUNCTIONS = os.path.join(HERE, "functions.tsv")
TWINS = os.path.join(HERE, "twins.tsv")
TWINS_PROGRAMS = os.path.join(HERE, "twins_programs.tsv")
SCOPES = ("missions", "programs")
GROUP_PREFIX = {"missions": "g", "programs": "p"}
DEFAULT_SYMS = ["ac1_markup.tsv"]      # match_fn.py's implicit table when a row passes no --syms
SYM_WINDOW = 0x1000                    # a table name may be up to this far below a slot target
MANIFEST = os.path.join(REPO, "re", "match", "manifest.tsv")
REPORT = os.path.join(REPO, "re", "match", "twin_replay.tsv")
MATCH_FN = os.path.join(REPO, "tools", "match", "match_fn.py")

HI_LO, HI_HI = 0x8000, 0x8020          # lui immediates that are RAM high halves
LO_OPS = {0x09, 0x0D,                  # addiu, ori
          0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,   # lb lh lwl lw lbu lhu lwr
          0x28, 0x29, 0x2A, 0x2B, 0x2E,               # sb sh swl sw swr
          0x32, 0x3A}                                  # lwc2 swc2
WRITES_RT = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
             0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26}


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def written_reg(w):
    """GPR an instruction writes (0 = none), for HI tracking."""
    op = w >> 26
    if op == 0:
        f = w & 0x3F
        if f in (0x08, 0x18, 0x19, 0x1A, 0x1B, 0x11, 0x13, 0x0C, 0x0D):  # jr mult div mthi mtlo syscall break
            return 0
        if f == 0x09:                                                    # jalr
            return (w >> 11) & 31
        return (w >> 11) & 31
    if op == 3:
        return 31
    if op in WRITES_RT:
        return (w >> 16) & 31
    if op == 0x12 and ((w >> 21) & 31) in (0, 2):                        # mfc2 / cfc2
        return (w >> 16) & 31
    if op == 0x10 and ((w >> 21) & 31) == 0:                             # mfc0
        return (w >> 16) & 31
    return 0


def mask_function(words, va0):
    """(masked tuple, slots) where slots = [(index, kind, target)] in order.

    kind: J (jal/j target), HI (lui of a pair), LO (low half; target = composed
    address), HI* (lui whose pair was not found; target = hi << 16)."""
    masked = list(words)
    slots = []
    hi = {}                    # reg -> (lui index, hi value, used?)
    for i, w in enumerate(words):
        op = w >> 26
        if op in (2, 3):
            masked[i] = w & 0xFC000000
            slots.append((i, "J", ((va0 + 4 * i) & 0xF0000000) | ((w & 0x03FFFFFF) << 2)))
            continue
        rs, rt = (w >> 21) & 31, (w >> 16) & 31
        if op == 0x0F:
            imm = w & 0xFFFF
            if HI_LO <= imm <= HI_HI:
                masked[i] = w & 0xFFFF0000
                hi[rt] = [i, imm, False]
                slots.append((i, "HI", imm << 16))
                continue
            hi.pop(rt, None)
            continue
        if op in LO_OPS and rs in hi:
            masked[i] = w & 0xFFFF0000
            h = hi[rs]
            h[2] = True
            tgt = ((h[1] << 16) + (s16(w & 0xFFFF) if op != 0x0D else (w & 0xFFFF))) & 0xFFFFFFFF
            slots.append((i, "LO", tgt))
            if op in WRITES_RT and rt == rs:
                del hi[rs]
            elif op in WRITES_RT:
                hi.pop(rt, None)
            continue
        if op == 0 and (w & 0x3F) in (0x20, 0x21):
            # addu rd, rs, rt with one operand a pending HI: `lui at,%hi(t);
            # addu at,at,v0; lw v0,%lo(t)(at)` (switch tables, indexed arrays)
            rd, src = (w >> 11) & 31, (rs if rs in hi else rt if rt in hi else None)
            if src is not None and not (rs in hi and rt in hi) and rd:
                hi[rd] = hi[src] if rd == src else list(hi[src])
                continue
        r = written_reg(w)
        if r:
            hi.pop(r, None)
    return tuple(masked), slots


# ---------------------------------------------------------------------------
# reading the inventory and images
# ---------------------------------------------------------------------------
def scope_of(unit):
    """missions (fdatNNN) | programs (exe, ovl201..204) for a normalized unit name."""
    return "missions" if unit.startswith("fdat") else "programs"


def read_functions(path, scope="missions"):
    """functions.tsv rows of one scope; PsyQ library code (is_psyq 1) is left out."""
    rows = []
    with open(path, encoding="utf-8") as fh:
        header = fh.readline().rstrip("\r\n").split("\t")
        for line in fh:
            p = line.rstrip("\r\n").split("\t")
            r = dict(zip(header, p))
            if scope_of(r["unit"]) == scope and r.get("is_psyq", "0") != "1":
                rows.append(r)
    return rows


_images = {}


def image(unit):
    if unit not in _images:
        u = units.resolve(unit)
        with open(u.image_path, "rb") as fh:
            _images[unit] = (fh.read(), u.base, u.image_path)
    return _images[unit]


def func_words(unit, vaddr, size):
    data, base, _ = image(unit)
    off = vaddr - base
    if off < 0 or off + size > len(data):
        raise ValueError("%s 0x%08X+%d outside image" % (unit, vaddr, size))
    return [int.from_bytes(data[off + k:off + k + 4], "little") for k in range(0, size, 4)]


def entry_no(unit):
    """Sort key of a unit: exe first, then ovl201..204 / fdatNNN by number."""
    if unit == "exe":
        return -1
    return int(re.sub(r"\D", "", unit))


def analyse(functions_path, scope="missions"):
    """[(unit, vaddr, size, masked, slots)] for every row of the scope."""
    out = []
    for r in read_functions(functions_path, scope):
        unit, va, size = r["unit"], int(r["vaddr"], 16), int(r["size"])
        words = func_words(unit, va, size)
        masked, slots = mask_function(words, va)
        out.append((unit, va, size, masked, slots))
    return out


def build_groups(funcs):
    by = defaultdict(list)
    for f in funcs:
        by[f[3]].append(f)
    groups = []
    for masked, mem in by.items():
        mem.sort(key=lambda f: (entry_no(f[0]), f[1]))
        groups.append(mem)
    # biggest groups first, then by representative
    groups.sort(key=lambda g: (-len(g), -g[0][2], entry_no(g[0][0]), g[0][1]))
    return groups


def slot_deltas(rep_slots, mem_slots):
    """(differing [(rep_tgt, mem_tgt)] unique in order, n equal slots)."""
    diff, seen, same = [], set(), 0
    for (i, k, a), (_j, _k2, b) in zip(rep_slots, mem_slots):
        if k == "HI":
            continue            # the LO slot carries the composed address
        if a == b:
            same += 1
        elif (a, b) not in seen:
            seen.add((a, b))
            diff.append((a, b))
    return diff, same


def fmt_delta(a, b):
    d = b - a
    return "0x%08X>0x%08X(%s0x%X)" % (a, b, "+" if d >= 0 else "-", abs(d))


def cmd_group(args):
    scopes = SCOPES if args.scope == "all" else (args.scope,)
    if args.out and len(scopes) > 1:
        sys.exit("twins.py group: --out needs a single --scope")
    for sc in scopes:
        group_scope(args.functions, sc, args.out or (TWINS if sc == "missions" else TWINS_PROGRAMS))
    return 0


def group_scope(functions_path, scope, out):
    funcs = analyse(functions_path, scope)
    groups = build_groups(funcs)
    prefix = GROUP_PREFIX[scope]
    lines = ["# group\trep_unit\trep_vaddr\tunit\tvaddr\tsize\tn_members\tsame_slots\tdiff_slots\tdeltas"
             " (REP>MEMBER(+delta), relocation targets that differ from the representative)"]
    for gi, g in enumerate(groups, 1):
        rep = g[0]
        for m in g:
            diff, same = slot_deltas(rep[4], m[4])
            lines.append("\t".join([
                "%s%04d" % (prefix, gi), rep[0], "0x%08X" % rep[1], m[0], "0x%08X" % m[1], str(m[2]),
                str(len(g)), str(same), str(len(diff)), ",".join(fmt_delta(a, b) for a, b in diff)]))
    with open(out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(lines) + "\n")
    sizes = Counter(len(g) for g in groups)
    print("%s: %d functions in %d units -> %d groups (masked bodies)"
          % (scope, len(funcs), len({f[0] for f in funcs}), len(groups)))
    print("functions with a twin in another unit: %d"
          % sum(len(g) for g in groups if len({m[0] for m in g}) > 1))
    print("group size distribution (members: groups):")
    for k in sorted(sizes):
        print("  %3d: %d" % (k, sizes[k]))
    print("wrote %s" % os.path.relpath(out, os.getcwd()))
    return 0


# ---------------------------------------------------------------------------
# replay
# ---------------------------------------------------------------------------
def read_twins(path):
    """{(unit, vaddr): group}, {group: [(unit, vaddr, size)]}"""
    key2g, g2m = {}, defaultdict(list)
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or not line.strip():
                continue
            p = line.rstrip("\r\n").split("\t")
            g, unit, va, size = p[0], p[3], int(p[4], 16), int(p[5])
            key2g[(unit, va)] = g
            g2m[g].append((unit, va, size))
    return key2g, g2m


def parse_extra(extra):
    """(defsyms [(name, addr)], rodata or None, other args)."""
    toks = shlex.split(extra) if extra else []
    defs, rodata, other = [], None, []
    i = 0
    while i < len(toks):
        t = toks[i]
        if t == "--defsym":
            n, a = toks[i + 1].split("=", 1)
            defs.append((n, int(a, 0)))
            i += 2
        elif t.startswith("--defsym="):
            n, a = t.split("=", 1)[1].split("=", 1)
            defs.append((n, int(a, 0)))
            i += 1
        elif t == "--rodata":
            rodata = int(toks[i + 1], 0)
            i += 2
        elif t == "--base":
            i += 2              # replaced by the member's base
        else:
            other.append(t)
            i += 1
    return defs, rodata, other


def map_addr(addr, rep_slots, mem_slots, rep_unit):
    """Member address for a symbol at `addr` in the matched function's unit."""
    best = None
    for (_i, k, a), (_j, _k, b) in zip(rep_slots, mem_slots):
        if k in ("HI",):
            continue
        off = a - addr
        if 0 <= off < 0x1000 and (best is None or off < best[0]):
            best = (off, b - a)
    if best is not None:
        return addr + best[1], None
    data, base, _ = image(rep_unit)
    if base <= addr < base + len(data):
        return None, "no relocation slot targets 0x%08X (inside %s)" % (addr, rep_unit)
    return addr, None


def run_oracle(argv):
    res = subprocess.run([sys.executable, MATCH_FN] + argv, cwd=REPO,
                         capture_output=True, text=True, errors="replace")
    lines = [ln for ln in res.stdout.splitlines() if ln.strip()]
    line = lines[-1] if lines else ("(no output) " + res.stderr.strip()[-200:])
    return res.returncode, line


def table_syms(files):
    """[(addr, name)] sorted, from symbol TSVs parsed the way match_fn.load_symbols does."""
    import drive                                   # lazy: drive imports twins lazily too
    return sorted((a, n) for n, a in drive.table_syms(files).items())


def row_tables(other):
    """The symbol tables match_fn.py reads for a row: its --syms, else the implicit markup."""
    files, i = [], 0
    while i < len(other):
        if other[i] == "--syms" and i + 1 < len(other):
            files.append(other[i + 1])
            i += 2
            continue
        if other[i].startswith("--syms="):
            files.append(other[i].split("=", 1)[1])
        i += 1
    if not files and "--no-markup" not in other:
        files = list(DEFAULT_SYMS)
    return tuple(files)


def table_moves(tsyms, rep_slots, mem_slots, skip):
    """[(name, member address)] for table names whose relocation slot moved.

    A table name at A is tied to the slot with the smallest offset (target - A)
    in [0, SYM_WINDOW); it is emitted only when that slot's target differs in
    the member.  `skip` = names the row already passes as --defsym."""
    addrs = [a for a, _n in tsyms]
    best = {}                                      # name -> (off, A, delta)
    for (_i, k, a), (_j, _k, b) in zip(rep_slots, mem_slots):
        if k == "HI":
            continue
        lo = bisect.bisect_left(addrs, a - SYM_WINDOW + 1)
        hi = bisect.bisect_right(addrs, a)
        for idx in range(lo, hi):
            A, n = tsyms[idx]
            if n in skip:
                continue
            cur = best.get(n)
            if cur is None or a - A < cur[0]:
                best[n] = (a - A, A, b - a)
    return sorted((n, (A + d) & 0xFFFFFFFF) for n, (_off, A, d) in best.items() if d)


def is_original(r):
    """A manifest row whose source is its own unit's func_<ADDR>.c (not a replayed twin)."""
    try:
        unit = units.norm(r[1] or "exe")
        va = int(r[0], 16)
    except (ValueError, units.UnitError):
        return False
    return (r[2].split("/")[0] == unit
            and os.path.basename(r[2]).lower() == "func_%08x.c" % va)


def report_key(fields):
    """Sort key of a report row: (source, unit order, vaddr)."""
    try:
        return (fields[1], entry_no(units.norm(fields[3])), int(fields[4], 16))
    except (ValueError, IndexError, units.UnitError):
        return (fields[1] if len(fields) > 1 else "", 0, 0)


def cmd_replay(args):
    scopes = SCOPES if args.scope == "all" else (args.scope,)
    key2g, g2m = {}, defaultdict(list)
    for sc in scopes:
        path = args.twins if sc == "missions" else args.twins_programs
        if not os.path.isfile(path):
            print("note: %s not found (run twins.py group --scope %s)" % (path, sc))
            continue
        k2g, gm = read_twins(path)
        key2g.update(k2g)
        for g, mem in gm.items():
            g2m[g].extend(mem)
    header, rows = merge_manifest.parse(open(args.manifest, encoding="utf-8").read())
    have = set()
    for r in rows:
        if r[0].startswith("0x"):
            try:
                have.add((units.norm(r[1] or "exe"), int(r[0], 16)))
            except units.UnitError:
                pass
    # originals: rows whose source lives in that unit's own directory and is named after
    # that row's address (a group can have several members in one unit, e.g. fdat004
    # func_801C6734.c also matches 0x801C6A28 and 0x801C6D80: those are twins)
    origs = [r for r in rows if is_original(r) and scope_of(units.norm(r[1] or "exe")) in scopes]
    tables = {}
    jobs = []
    targeted = set()
    for r in origs:
        unit, va, src, size, extra = units.norm(r[1] or "exe"), int(r[0], 16), r[2], int(r[3]), r[4]
        sc = scope_of(unit)
        g = key2g.get((unit, va))
        if g is None:
            print("note: %s 0x%08X is not in the %s twins file (re-run twins.py group)" % (unit, va, sc))
            continue
        rep_words = func_words(unit, va, size)
        _m, rep_slots = mask_function(rep_words, va)
        defs, rodata, other = parse_extra(extra)
        tsyms = []
        if sc == "programs":
            tf = row_tables(other)
            if tf not in tables:
                tables[tf] = table_syms(tf)
            tsyms = tables[tf]
        for (munit, mva, msize) in g2m[g]:
            # one job per member: a group with several matched originals (clones matched
            # independently) replays only the first original onto each other member
            if (munit, mva) == (unit, va) or (munit, mva) in targeted:
                continue
            targeted.add((munit, mva))
            job = {"group": g, "src": src, "from": "%s 0x%08X" % (unit, va),
                   "unit": munit, "col": munit, "vaddr": mva, "size": msize, "extra": "", "err": None,
                   "already": (munit, mva) in have}
            try:
                mu = units.resolve(munit)
            except units.UnitError as e:
                job["err"] = "ERROR: %s" % e
                jobs.append(job)
                continue
            job["col"] = mu.manifest_unit
            _m2, mem_slots = mask_function(func_words(munit, mva, msize), mva)
            mdefs, errs = [], []
            for n, a in defs:
                b, err = map_addr(a, rep_slots, mem_slots, unit)
                if err:
                    errs.append("%s: %s" % (n, err))
                else:
                    mdefs.append((n, b))
            if tsyms:
                mdefs += table_moves(tsyms, rep_slots, mem_slots, {n for n, _a in defs})
            mrod = None
            if rodata is not None:
                mrod, err = map_addr(rodata, rep_slots, mem_slots, unit)
                if err:
                    errs.append("--rodata: " + err)
            ex = ["--base", "0x%08X" % mu.base] if sc == "missions" else []
            if mrod is not None:
                ex += ["--rodata", "0x%08X" % mrod]
            for n, b in mdefs:
                ex += ["--defsym", "%s=0x%08X" % (n, b)]
            ex += other
            job["extra"] = " ".join(ex)
            sel = ["--bin", mu.image_path] if sc == "missions" else list(mu.oracle_flags)
            job["argv"] = (["0x%08X" % mva, os.path.join(os.path.dirname(args.manifest), src)]
                           + sel + ex + ["--expect-size", str(msize), "-q"])
            if errs:
                job["err"] = "ERROR: " + "; ".join(errs)
            jobs.append(job)

    todo = [j for j in jobs if not j["err"]]
    print("%d matched original row(s) in scope %s, %d twin(s) to replay"
          % (len(origs), "+".join(scopes), len(jobs)))
    if args.dry_run:
        for j in jobs:
            print("%s  %s 0x%08X  <- %s  %s" % (j["group"], j["unit"], j["vaddr"], j["src"], j["err"] or j["extra"]))
        return 0
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for j, (rc, line) in zip(todo, ex.map(lambda j: run_oracle(j["argv"]), todo)):
            j["rc"], j["line"] = rc, line
    for j in jobs:
        if j["err"]:
            j["rc"], j["line"] = 2, j["err"]
        j["result"] = "MATCH" if j["rc"] == 0 and j["line"].startswith("MATCH") else \
            ("ERROR" if j["rc"] == 2 or j["line"].startswith("ERROR") else "MISMATCH")

    added = 0
    new_rows = list(rows)
    for j in jobs:
        if j["result"] == "MATCH" and not j["already"]:
            new_rows.append(["0x%08X" % j["vaddr"], j["col"], j["src"], str(j["size"]), j["extra"]])
            added += 1
    recorded = 0
    if not args.no_ledger:
        lrows = ledger.load()
        for j in jobs:
            if j["result"] == "MATCH" and ledger.state(j["unit"], "0x%08X" % j["vaddr"], lrows) != "match":
                r = ledger.record("match", j["unit"], "0x%08X" % j["vaddr"], 0,
                                  "twin of %s (%s)" % (j["from"], j["src"]))
                lrows[(r["unit"], r["vaddr"])] = r
                recorded += 1
    merged = merge_manifest.merge(new_rows)
    with open(args.manifest, "wb") as fh:
        fh.write(((header or merge_manifest.HEADER) + "\n" +
                  "".join("\t".join(r) + "\n" for r in merged)).encode("utf-8"))

    # report: this run's rows plus the other scope's rows as they were
    keep = []
    if len(scopes) < len(SCOPES) and os.path.isfile(args.report):
        with open(args.report, encoding="utf-8") as fh:
            for line in fh:
                p = line.rstrip("\r\n").split("\t")
                if line.startswith("#") or len(p) < 5:
                    continue
                try:
                    if scope_of(units.norm(p[3])) not in scopes:
                        keep.append(p)
                except units.UnitError:
                    keep.append(p)
    out = keep + [[j["group"], j["src"], j["from"], j["col"], "0x%08X" % j["vaddr"],
                   str(j["size"]), j["result"], j["line"], j["extra"]] for j in jobs]
    out.sort(key=report_key)
    with open(args.report, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("# twin replay (tools/loop/twins.py replay): every clone of a matched function,"
                 " compiled from the shared C file\n")
        fh.write("# group\tsource\tmatched_as\tunit\tvaddr\tsize\tresult\toracle_final_line\textra\n")
        for p in out:
            fh.write("\t".join(p) + "\n")
    c = Counter(j["result"] for j in jobs)
    print("replayed %d twin(s): %s" % (len(jobs), ", ".join("%s %d" % kv for kv in sorted(c.items()))))
    print("manifest: %d row(s) added (%d already present)" % (added, sum(1 for j in jobs if j["already"])))
    if not args.no_ledger:
        print("ledger: %d match row(s) recorded in %s" % (recorded, os.path.relpath(ledger.ledger_path(), os.getcwd())))
    for j in jobs:
        if j["result"] != "MATCH":
            print("  NOT MATCHED  %s 0x%08X  %s  %s" % (j["unit"], j["vaddr"], j["src"], j["line"]))
    print("wrote %s" % os.path.relpath(args.report, os.getcwd()))
    return 0 if c.get("MATCH", 0) == len(jobs) else 1


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd")
    p = sub.add_parser("group", help="group functions by masked body -> twins.tsv / twins_programs.tsv")
    p.add_argument("--scope", choices=SCOPES + ("all",), default="all")
    p.add_argument("--functions", default=FUNCTIONS)
    p.add_argument("--out", default=None, help="output file (one --scope only)")
    p.set_defaults(func=cmd_group)
    p = sub.add_parser("replay", help="replay matched C files against their twins")
    p.add_argument("--scope", choices=SCOPES + ("all",), default="all")
    p.add_argument("--manifest", default=MANIFEST)
    p.add_argument("--twins", default=TWINS)
    p.add_argument("--twins-programs", default=TWINS_PROGRAMS)
    p.add_argument("--report", default=REPORT)
    p.add_argument("--dry-run", action="store_true", help="print the derived oracle flags only")
    p.add_argument("--no-ledger", action="store_true", help="do not record the twins in the ledger")
    p.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    p.set_defaults(func=cmd_replay)
    args = ap.parse_args(argv)
    if not args.cmd:
        ap.print_help()
        return 2
    args.manifest = os.path.abspath(getattr(args, "manifest", MANIFEST))
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
