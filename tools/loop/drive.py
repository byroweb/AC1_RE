#!/usr/bin/env python3
"""drive.py - the driver half of the matching loop: everything except writing the C.

    drive.py next   [--unit U] [--queue FILE] [--shard k/n] [--max-score S] [--no-m2c]
                    [--dry-run] [--re-dir DIR]
    drive.py try    <unit> <vaddr> [--defsym NAME=ADDR]... [--rodata VA] [--no-rodata-align-fix]
                    [--no-auto-defsym]
                    [--commit] [--keep] [--re-dir DIR]
    drive.py giveup <unit> <vaddr> <reason...>
    drive.py status [--queue FILE]
    drive.py verify [--manifest FILE] [-j N] [-q]

next    prints a work package for the next open function (exit 3 when nothing is
        left): the functions.tsv row and score, the retail disassembly, the GAS
        that m2c reads, the m2c draft, the exact oracle command line and the C
        path.  If the C file does not exist it writes a skeleton with the draft
        in a comment; that is the only file `next` writes.  With --queue the
        rows come from a matcher queue (queue-A.tsv) and $AC1_LEDGER must point
        at that matcher's own ledger; otherwise rank.candidates() is used,
        optionally sharded by --shard k/n (int(vaddr,16)//4 % n == k).
try     runs the oracle on re/match/<unit>/func_<ADDR>.c with the unit's flags
        and --expect-size from functions.tsv (+ --rodata from jt_addrs when the
        function has a jump table and you pass none; --no-rodata-align-fix is passed
        through to the oracle and kept in the manifest row).  Unresolved symbols named
        func_<ADDR> / D_<ADDR> are their own address: they are added as
        --defsym automatically and the oracle is rerun once.  Every try appends
        one line to re/match/<unit>/func_<ADDR>.attempts (gitignored).  Prints
        the oracle's diff and ONE hint picked from the diff-pattern table of
        .claude/skills/match-function/SKILL.md.  Exit 0 = clean MATCH (right
        size, no masked word); 1 = MISMATCH or masked MATCH; 2 = ERROR.
        On a clean MATCH the manifest row is appended (deduped), the ledger
        gets a match row, and the commit command is printed (--commit runs it).
giveup  records a give-up in the ledger (attempts = sidecar lines) and prepends
        /* NOT MATCHED: ... */ to the C file.
status  ledger state, attempts and last result per function (queue or ledger).
verify  re-runs the oracle on every manifest row (Python check_all.sh); exit 0
        only if every row is MATCH.  -j N runs N oracles at once (default: CPU
        count; output stays in manifest order), -q prints only FAIL rows.

Environment: AC1_LEDGER (ledger file, see ledger.py), M2C_DIR (m2c checkout,
default ~/dev/m2c or ~/Applications/m2c), MIPS_PREFIX, AC1_BIN / AC1_DISC_MAP
(images), plus whatever tools/match/match_fn.py reads (AC1_CC1, AC1_MASPSX...).
"""
import argparse
import datetime
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import ledger  # noqa: E402
import rank    # noqa: E402
import units   # noqa: E402

MATCH_FN = os.path.join(REPO, "tools", "match", "match_fn.py")
OBJDUMP2M2C = os.path.join(REPO, "tools", "ghidra", "objdump2m2c.py")
RE_MATCH = os.path.join(REPO, "re", "match")
MANIFEST = os.path.join(RE_MATCH, "manifest.tsv")
MANIFEST_HEADER = ("# vaddr\tovl\tsource\tsize\textra match_fn.py args (one line per matched "
                   "function; re/match/check_all.sh re-verifies every row)")
GIVEUP_MARK = "NOT MATCHED"
COMMIT_TRAILER = "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
AUTO_SYM = re.compile(r"^(func|D)_([0-9A-Fa-f]{8})$")
# A source that includes the shared headers resolves names from both tables. Any --syms
# switches off match_fn's implicit ac1_markup.tsv, so the markup is passed explicitly too.
AC1_INCLUDE = re.compile(r'^\s*#\s*include\s*"ac1/ac1\.h"', re.M)
AC1_SYMS = ["include/ac1/symbols.tsv", "ac1_markup.tsv"]
QUEUE_COLS = ["unit", "vaddr", "name", "size", "instructions", "branches", "calls",
              "jump_tables", "gte_ops", "stack_size", "jt_addrs", "score"]


# ---------------------------------------------------------------------------
# small helpers
# ---------------------------------------------------------------------------
def die(msg, code=2):
    print("drive.py: " + msg, file=sys.stderr)
    sys.exit(code)


def rel(path):
    """Path relative to the repo root, forward slashes (for printing / git add)."""
    try:
        r = os.path.relpath(path, REPO)
    except ValueError:
        r = path
    if r.startswith(".."):
        r = path
    return r.replace("\\", "/")


def fn_name(vaddr):
    if isinstance(vaddr, int):
        return "func_%08X" % vaddr
    return "func_%s" % ledger.norm_vaddr(vaddr)[2:]


def c_path(re_dir, unit, vaddr):
    return os.path.join(re_dir, unit, fn_name(vaddr) + ".c")


def sidecar_path(re_dir, unit, vaddr):
    return os.path.join(re_dir, unit, fn_name(vaddr) + ".attempts")


def read_sidecar(path):
    if not os.path.isfile(path):
        return []
    with open(path, encoding="utf-8") as fh:
        return [ln.rstrip("\r\n") for ln in fh if ln.strip()]


def append_sidecar(path, result, first_diff, diff_words):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    when = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    with open(path, "a", encoding="utf-8", newline="\n") as fh:
        fh.write("%s\t%s\t%s\t%s\n" % (when, result,
                                       "-" if first_diff is None else "0x%x" % first_diff,
                                       diff_words if diff_words is not None else "-"))
    return len(read_sidecar(path))


def is_given_up(cpath):
    if not os.path.isfile(cpath):
        return False
    with open(cpath, encoding="utf-8", errors="replace") as fh:
        return GIVEUP_MARK in fh.read(400)


def load_rows():
    """{(unit, vaddr): functions.tsv row (+score)}."""
    out = {}
    for r in rank.load_functions():
        r = dict(r)
        r["score"] = rank.score(r)
        out[(r["unit"], r["vaddr"])] = r
    return out


def read_queue(path):
    rows = []
    with open(path, encoding="utf-8") as fh:
        header = None
        for line in fh:
            line = line.rstrip("\r\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if header is None:
                header = parts
                continue
            row = dict(zip(header, parts))
            for k in ("size", "instructions", "branches", "calls", "jump_tables",
                      "gte_ops", "stack_size"):
                if k in row:
                    row[k] = int(row[k] or 0)
            row["score"] = float(row.get("score") or 0)
            row["unit"] = ledger.norm_unit(row["unit"])
            row["vaddr"] = ledger.norm_vaddr(row["vaddr"])
            rows.append(row)
    return rows


def row_line(row):
    return "\t".join(QUEUE_COLS) + "\n" + "\t".join(
        ("%.3f" % row[k]) if k == "score" else str(row.get(k, "")) for k in QUEUE_COLS)


def unit_or_die(unit, need_image=True):
    try:
        return units.resolve(unit, need_image=need_image)
    except (units.UnitError, ValueError) as e:
        die(str(e))


def objdump_text(u, vaddr, size, march):
    objdump = units.find_objdump()
    cmd = [objdump, "-D", "-b", "binary", "-m", march, "-EL", "--adjust-vma=0x%X" % u.base,
           "--start-address=0x%X" % vaddr, "--stop-address=0x%X" % (vaddr + size), u.image_path]
    res = subprocess.run(cmd, capture_output=True, text=True, errors="replace")
    if res.returncode != 0:
        die("objdump failed: %s" % res.stderr.strip())
    return res.stdout


def listing_only(dump):
    return "\n".join(ln for ln in dump.splitlines()
                     if re.match(r"^\s*[0-9a-f]+:\s+[0-9a-f]{8}\s", ln))


def gas_text(u, vaddr, size):
    dump = objdump_text(u, vaddr, size, "mips:isa32")
    res = subprocess.run([sys.executable, OBJDUMP2M2C, "0x%X" % vaddr, fn_name(vaddr)],
                         input=dump, capture_output=True, text=True)
    if res.returncode != 0:
        die("objdump2m2c.py failed: %s" % res.stderr.strip())
    return res.stdout, res.stderr.strip()


def m2c_cmd():
    d = os.environ.get("M2C_DIR")
    if not d:
        for cand in (os.path.join(os.path.expanduser("~"), "dev", "m2c"),
                     os.path.join(os.path.expanduser("~"), "Applications", "m2c")):
            if os.path.isdir(cand):
                d = cand
                break
    if not d or not os.path.isfile(os.path.join(d, "m2c.py")):
        return None, "m2c not found (set M2C_DIR to the m2c checkout)"
    py = None
    for cand in (os.path.join(d, ".venv", "Scripts", "python.exe"),
                 os.path.join(d, ".venv", "bin", "python")):
        if os.path.isfile(cand):
            py = cand
            break
    return [py or sys.executable, os.path.join(d, "m2c.py")], None


def m2c_draft(gas):
    cmd, err = m2c_cmd()
    if not cmd:
        return None, err
    fd, path = tempfile.mkstemp(suffix=".s", prefix="drive_")
    with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(gas)
    base = cmd + ["-t", "mips-gcc-c", "--valid-syntax", "--deterministic-vars", "--stop-on-error"]
    errs = []
    try:
        for extra in ([], ["--no-andor"]):
            res = subprocess.run(base + extra + [path], capture_output=True, text=True, errors="replace")
            if res.returncode == 0 and res.stdout.strip():
                return res.stdout, None
            errs.append((res.stderr or res.stdout).strip()[-1500:])
    finally:
        try:
            os.unlink(path)
        except OSError:
            pass
    return None, "m2c failed (also with --no-andor):\n" + "\n---\n".join(errs)


# ---------------------------------------------------------------------------
# oracle
# ---------------------------------------------------------------------------
def uses_ac1_h(cpath):
    try:
        with open(cpath, encoding="utf-8", errors="replace") as fh:
            return bool(AC1_INCLUDE.search(fh.read()))
    except OSError:
        return False


def syms_args(cpath):
    """--syms for a source that includes ac1/ac1.h (repo-relative: the oracle runs in REPO)."""
    out = []
    if uses_ac1_h(cpath):
        for f in AC1_SYMS:
            out += ["--syms", f]
    return out


def table_syms(files=AC1_SYMS):
    """name -> address from the --syms tables, parsed the way match_fn.load_symbols does."""
    syms = {}
    for f in files:
        try:
            fh = open(os.path.join(REPO, f), encoding="utf-8", errors="replace")
        except OSError:
            continue
        with fh:
            for line in fh:
                parts = line.rstrip("\r\n").split("\t")
                if len(parts) >= 3 and re.fullmatch(r"[0-9a-fA-F]{6,8}", parts[1]):
                    syms.setdefault(parts[2], int(parts[1], 16))
                elif len(parts) >= 2 and re.fullmatch(r"(0x)?[0-9a-fA-F]{6,8}", parts[0]):
                    syms.setdefault(parts[1], int(parts[0], 16))
    return syms


def drop_table_defsyms(defsyms, syms):
    """Drop NAME=ADDR entries a table already resolves to the same address; a --defsym that
    disagrees with the table still wins in match_fn, so it is kept (with a warning)."""
    keep = []
    for d in defsyms:
        n, _, v = d.partition("=")
        try:
            addr = int(v, 0)
        except ValueError:
            addr = None
        if n in syms and syms[n] == addr:
            continue
        if n in syms:
            print("warning: --defsym %s overrides the table address 0x%08X" % (d, syms[n]))
        keep.append(d)
    return keep


def oracle_argv(u, vaddr, cpath, size, rodata=None, defsyms=(), keep=None, row=None, flags=()):
    argv = ["0x%08X" % vaddr, rel(cpath)] + list(u.oracle_flags) + list(flags)
    if size:
        argv += ["--expect-size", str(size)]
    if rodata:
        argv += ["--rodata", rodata]
    argv += syms_args(cpath)
    for d in defsyms:
        argv += ["--defsym", d]
    if keep:
        argv += ["--keep", keep]
    return argv


def oracle_cmdline(argv):
    return "python tools/match/match_fn.py " + " ".join(shlex.quote(a) for a in argv)


def run_oracle(argv):
    """-> (exit code, facts dict or None, stdout text without the JSON block, stderr)."""
    res = subprocess.run([sys.executable, MATCH_FN] + argv + ["--json", "-"],
                         cwd=REPO, capture_output=True, text=True, errors="replace")
    lines = res.stdout.splitlines()
    facts, text = None, lines
    close = None
    for i in range(len(lines) - 1, -1, -1):
        if lines[i] == "}":
            close = i
            break
    if close is not None:
        for j in range(close - 1, -1, -1):
            if lines[j] == "{":
                try:
                    facts = json.loads("\n".join(lines[j:close + 1]))
                    text = lines[:j] + lines[close + 1:]
                except ValueError:
                    facts = None
                break
    return res.returncode, facts, "\n".join(text), res.stderr


DIFF_ROW = re.compile(r"^([!m ]) ([0-9a-f]{8})  (.*?) \| (.*)$")


def diff_rows(text):
    rows = []
    for ln in text.splitlines():
        m = DIFF_ROW.match(ln)
        if m:
            rows.append((m.group(1), int(m.group(2), 16), m.group(3).strip(), m.group(4).strip()))
    return rows


def mnem(s):
    return s.split()[0] if s.split() else ""


def regs_norm(s):
    return re.sub(r"\b(v[01]|a[0-3]|t[0-9]|s[0-7])\b", "R", s)


def imm_norm(s):
    return re.sub(r"-?\b(0x[0-9a-f]+|\d+)\b", "N", s)


INVERSE = {"beq": "bne", "bne": "beq", "beqz": "bnez", "bnez": "beqz", "blez": "bgtz",
           "bgtz": "blez", "bltz": "bgez", "bgez": "bltz"}
SIGN_PAIRS = {("lh", "lhu"), ("lhu", "lh"), ("lb", "lbu"), ("lbu", "lb"), ("lw", "lh"),
              ("lh", "lw"), ("lw", "lhu"), ("lhu", "lw"), ("lw", "lb"), ("lb", "lw"),
              ("lw", "lbu"), ("lbu", "lw"), ("slt", "sltu"), ("sltu", "slt"),
              ("slti", "sltiu"), ("sltiu", "slti")}
MEM = {"lw", "lh", "lhu", "lb", "lbu", "sw", "sh", "sb", "lwl", "lwr", "swl", "swr"}
LO_IMM = MEM | {"addiu", "ori", "lui"}
SHIFTS = {"sll", "srl", "sra", "addu", "subu"}


def pick_hint(rc, facts, text, expect_size):
    """ONE sentence from the SKILL.md diff-pattern table, chosen by rule."""
    if facts is None or facts.get("result") == "ERROR":
        err = (facts or {}).get("error") or "(no JSON from the oracle)"
        return ("build error, not a near miss: %s - read the toolchain log above (syntax error, "
                "undeclared symbol/type, missing extern or #include)." % err)
    rows = diff_rows(text)
    diffs = [r for r in rows if r[0] == "!"]
    ours = facts.get("bytes") or 0
    first = facts.get("first_diff_offset")
    if facts.get("result") == "MATCH" and facts.get("masked_words"):
        return ("shape match only: %d word(s) masked for %s - give those symbols an address "
                "(--defsym NAME=0xADDR, or a name from ac1_markup.tsv)."
                % (facts["masked_words"], ", ".join(facts.get("unresolved_symbols") or [])))
    if expect_size and ours != expect_size:
        if not diffs:
            return ("0 differing words but size ours %d vs expected %d: your C is a strict prefix of "
                    "the retail function (statements missing at the end), or the size is wrong "
                    "(delay-slot nop counted twice / not at all)." % (ours, expect_size))
        return ("size differs: ours %d, retail %d bytes (%+d): a missing/extra statement, early "
                "return or stack frame difference - fix the first diff at 0x%x first."
                % (ours, expect_size, ours - expect_size, first if first is not None else 0))
    if not diffs:
        return None
    if all(mnem(a) == mnem(b) and regs_norm(a) == regs_norm(b) for _, _, a, b in diffs):
        return ("same instructions, registers swapped: locals declared/first used in a different "
                "order, or a temporary that should be a named local (or the reverse).")
    if sorted(a for _, _, a, _ in diffs) == sorted(b for _, _, _, b in diffs):
        return ("same instructions, different order: statement order in your source differs from "
                "retail (GCC 2.7.2 keeps the source order of side effects).")
    if sorted(regs_norm(a) for _, _, a, _ in diffs) == sorted(regs_norm(b) for _, _, _, b in diffs):
        return ("same instructions, different order (registers renamed to follow): statement order "
                "in your source differs from retail (GCC 2.7.2 keeps the source order of side "
                "effects) - swap the statements around the first diff.")
    if all(mnem(a) == mnem(b) and mnem(a) in MEM and imm_norm(a) == imm_norm(b) for _, _, a, b in diffs):
        return ("only load/store offsets differ: wrong field offset - fix the struct layout, do "
                "not cast pointer arithmetic.")
    if (len(diffs) <= 2 and all(mnem(a) == mnem(b) and mnem(a) in LO_IMM and imm_norm(a) == imm_norm(b)
                                for _, _, a, b in diffs) and facts.get("resolved_symbols")):
        return ("a lui / low-half immediate differs and everything else matches: a --defsym (or "
                "markup) address is wrong - recompute it from the retail lui + addiu/lw pair "
                "(negative low halves!).")
    for _, addr, a, b in diffs:
        if "sp,sp," in a or "sp,sp," in b or (mnem(a) == "sw" and "ra," in a) or (mnem(b) == "sw" and "ra," in b):
            return ("stack frame / epilogue differs at %08x: number of locals that live across "
                    "calls, or an s-register you use/do not use; a static, an extra parameter, "
                    "or a temporary retail spills." % addr)
    for _, addr, a, b in diffs:
        if mnem(a) == "jal" and mnem(b) == "jal":
            return ("jal target differs at %08x: wrong callee name/address - --defsym it or "
                    "check ac1_markup.tsv." % addr)
    for _, addr, a, b in diffs:
        if INVERSE.get(mnem(a)) == mnem(b):
            return ("inverted branch at %08x (%s vs %s): the condition is negated - swap the "
                    "if/else bodies or the comparison sense (== / !=, < / >=); if everything "
                    "drifts from here an early return is involved." % (addr, mnem(a), mnem(b)))
    for _, addr, a, b in diffs:
        if (a == "nop") != (b == "nop"):
            return ("nop count differs at %08x: load-delay slot - a loaded value is used right "
                    "after its load (or no longer is); move the use, or it is a different "
                    "variable; volatile changes this too." % addr)
    for _, addr, a, b in diffs:
        if (mnem(a), mnem(b)) in SIGN_PAIRS:
            return ("%s vs %s at %08x: wrong field type (signed/unsigned or width); plain char is "
                    "unsigned (-funsigned-char)." % (mnem(a), mnem(b), addr))
    for _, addr, a, b in diffs:
        if (mnem(a) == "andi") != (mnem(b) == "andi"):
            return ("andi 0xff/0xffff present on one side only at %08x: implicit narrowing - an "
                    "expression computed in a narrower type or stored to a u8/u16 field." % addr)
    for _, addr, a, b in diffs:
        if ({mnem(a), mnem(b)} & {"sll", "sra"}) and mnem(a) != mnem(b):
            return ("sll/sra pair present on one side only at %08x: sign extension of an s8/s16 "
                    "value, or a char/short parameter being promoted." % addr)
    for _, addr, a, b in diffs:
        if ({mnem(a), mnem(b)} & {"mult", "multu", "div", "divu"}) and ({mnem(a), mnem(b)} & SHIFTS):
            return ("mult/div vs shift-and-add at %08x: multiply/divide by a constant vs a "
                    "variable, or signed vs unsigned operands." % addr)
    for _, addr, a, b in diffs:
        if (mnem(a) == "jr" and "ra" not in a) != (mnem(b) == "jr" and "ra" not in b):
            return ("switch table (jr tN) on one side only at %08x: retail used switch with dense "
                    "cases vs an if chain (or the reverse); jump tables need --rodata." % addr)
    nwords = max(1, ours // 4)
    if len(diffs) > nwords // 2 and first is not None:
        return ("everything after 0x%x drifts (%d of %d words differ): a missing/extra early "
                "return or an inverted condition near the first diff - fix only the first "
                "difference and rerun." % (first, len(diffs), nwords))
    _, addr, a, b = diffs[0]
    return ("first diff at %08x: retail `%s` vs ours `%s` - see the diff-pattern table in "
            ".claude/skills/match-function/SKILL.md." % (addr, a, b))


# ---------------------------------------------------------------------------
# manifest
# ---------------------------------------------------------------------------
def read_manifest(path=MANIFEST):
    """-> (header line or None, [dict(vaddr, unit, src, size, extra)], crlf?)."""
    rows, header, crlf = [], None, False
    if not os.path.isfile(path):
        return header, rows, crlf
    with open(path, "rb") as fh:
        raw = fh.read().decode("utf-8", errors="replace")
    crlf = "\r\n" in raw
    for line in raw.splitlines():
        line = line.rstrip("\r")
        if not line.strip():
            continue
        if line.startswith("#"):
            if header is None:
                header = line
            continue
        parts = (line.split("\t") + ["", "", "", "", ""])[:5]
        rows.append({"vaddr": parts[0].strip(), "unit": parts[1].strip(), "src": parts[2].strip(),
                     "size": parts[3].strip(), "extra": parts[4].strip()})
    return header, rows, crlf


def manifest_has(rows, vaddr, manifest_unit):
    v = ledger.norm_vaddr(vaddr)
    for r in rows:
        try:
            if ledger.norm_vaddr(r["vaddr"]) == v and units.norm(r["unit"] or "exe") == units.norm(manifest_unit):
                return True
        except (ValueError, units.UnitError):
            continue
    return False


def manifest_append(vaddr, manifest_unit, src, size, extra, path=MANIFEST):
    header, rows, crlf = read_manifest(path)
    if manifest_has(rows, vaddr, manifest_unit):
        return False
    eol = "\r\n" if crlf else "\n"
    new = not os.path.isfile(path) or os.path.getsize(path) == 0
    with open(path, "a", encoding="utf-8", newline="") as fh:
        if new:
            fh.write(MANIFEST_HEADER + eol)
        fh.write("\t".join([ledger.norm_vaddr(vaddr), manifest_unit, src, str(size), extra]) + eol)
    return True


# ---------------------------------------------------------------------------
# next
# ---------------------------------------------------------------------------
def parse_shard(s):
    m = re.fullmatch(r"(\d+)/(\d+)", s or "")
    if not m or int(m.group(2)) < 1 or int(m.group(1)) >= int(m.group(2)):
        die("--shard wants k/n with 0 <= k < n")
    return int(m.group(1)), int(m.group(2))


def skeleton(cpath, row, draft):
    body = (draft or "/* no m2c draft */").replace("*/", "* /").rstrip("\n")
    lines = ["#include \"ac1/ac1.h\"",
             "/* %s %s  size %d  score %.3f  %s" % (row["unit"], row["vaddr"], row["size"],
                                                    row["score"], datetime.date.today().isoformat()),
             " * m2c draft:",
             " *"]
    lines += [" * " + ln if ln else " *" for ln in body.splitlines()]
    lines += [" */", "", "void %s(void)" % fn_name(row["vaddr"]), "{", "}", ""]
    os.makedirs(os.path.dirname(cpath), exist_ok=True)
    with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(lines))


def cmd_next(args):
    re_dir = os.path.abspath(args.re_dir)
    if args.queue:
        if not os.environ.get("AC1_LEDGER"):
            die("next --queue needs $AC1_LEDGER set to this matcher's own ledger file "
                "(the shared tools/loop/ledger.tsv is merged by the lead)", 2)
        rows = read_queue(args.queue)
        if args.unit:
            rows = [r for r in rows if r["unit"] == ledger.norm_unit(args.unit)]
        current = ledger.load()
        pool = [r for r in rows if ledger.state(r["unit"], r["vaddr"], current) == "open"]
    else:
        done = ledger.load(ledger.DEFAULT_LEDGER)
        done.update(ledger.load())
        unit = ledger.norm_unit(args.unit) if args.unit else None
        pool = rank.candidates(rank.load_functions(), done, unit)
        if args.shard:
            k, n = parse_shard(args.shard)
            pool = [r for r in pool if (int(r["vaddr"], 16) // 4) % n == k]
    if args.max_score is not None:
        pool = [r for r in pool if r["score"] <= args.max_score]
    pick = None
    for r in pool:
        if is_given_up(c_path(re_dir, r["unit"], r["vaddr"])):
            continue
        pick = r
        break
    if pick is None:
        print("queue exhausted: no open function left%s" % (" in " + args.queue if args.queue else ""))
        return 3

    unit = pick["unit"]
    vaddr = int(pick["vaddr"], 16)
    size = int(pick["size"])
    u = unit_or_die(unit)
    cpath = c_path(re_dir, unit, pick["vaddr"])
    print("== next: %s %s %s  size %d  score %.3f" % (unit, pick["vaddr"], pick["name"], size, pick["score"]))
    print(row_line(pick))
    print("\n-- retail disassembly (objdump -m mips:3000)")
    print(listing_only(objdump_text(u, vaddr, size, "mips:3000")))
    gas, warn = gas_text(u, vaddr, size)
    print("\n-- GAS for m2c (objdump2m2c.py)")
    if warn:
        print(warn)
    print(gas.rstrip("\n"))
    draft = None
    if not args.no_m2c:
        draft, err = m2c_draft(gas)
        print("\n-- m2c draft")
        print(draft.rstrip("\n") if draft else "m2c draft unavailable: " + err)
    rodata = pick["jt_addrs"].split(",")[0].strip() if int(pick["jump_tables"]) and pick.get("jt_addrs") else None
    if os.path.isfile(cpath):
        state = "exists (%d attempt(s) so far)" % len(read_sidecar(sidecar_path(re_dir, unit, pick["vaddr"])))
    elif args.dry_run:
        state = "absent (dry run: skeleton not written)"
    else:
        skeleton(cpath, pick, draft)
        state = "skeleton written"
    # the oracle line reflects the file's includes, so build it after the skeleton exists
    argv = oracle_argv(u, vaddr, cpath, size, rodata=rodata)
    print("\n-- oracle")
    print(oracle_cmdline(argv) + ("   # --rodata from jt_addrs: verify" if rodata else ""))
    print("python tools/loop/drive.py try %s %s%s" % (unit, pick["vaddr"],
                                                   (" --rodata " + rodata) if rodata else ""))
    print("\n-- C file")
    print("%s  %s" % (rel(cpath), state))
    return 0


# ---------------------------------------------------------------------------
# try
# ---------------------------------------------------------------------------
def cmd_try(args):
    unit = ledger.norm_unit(args.unit)
    vaddr_s = ledger.norm_vaddr(args.vaddr)
    vaddr = int(vaddr_s, 16)
    re_dir = os.path.abspath(args.re_dir)
    scratch = os.path.normpath(re_dir) != os.path.normpath(RE_MATCH)
    u = unit_or_die(unit)
    cpath = c_path(re_dir, unit, vaddr_s)
    if not os.path.isfile(cpath):
        die("no C file %s (run `drive.py next` for a skeleton)" % rel(cpath))
    row = load_rows().get((unit, vaddr_s))
    size = int(row["size"]) if row else None
    if row is None:
        print("warning: %s %s is not in functions.tsv: no --expect-size (size is unchecked)" % (unit, vaddr_s))
    rodata = args.rodata
    if not rodata and row and int(row["jump_tables"]) and row.get("jt_addrs"):
        rodata = row["jt_addrs"].split(",")[0].strip()
        print("note: --rodata %s taken from functions.tsv jt_addrs (verify it is the object's .rodata start)" % rodata)
    defsyms = list(args.defsym or [])
    flags = ["--no-rodata-align-fix"] if args.no_rodata_align_fix else []
    keep = None
    if args.keep:
        keep = os.path.join(tempfile.gettempdir(), "m_%s_%s" % (unit, vaddr_s[2:]))

    argv = oracle_argv(u, vaddr, cpath, size, rodata, defsyms, keep, flags=flags)
    print("$ " + oracle_cmdline(argv))
    rc, facts, text, err = run_oracle(argv)
    other = []
    if facts and facts.get("unresolved_symbols") and not args.no_auto_defsym:
        auto = []
        for s in facts["unresolved_symbols"]:
            m = AUTO_SYM.match(s)
            if m:
                auto.append("%s=0x%s" % (s, m.group(2).upper()))
            else:
                other.append(s)
        if auto:
            defsyms += auto
            print("auto --defsym: " + " ".join(auto))
            argv = oracle_argv(u, vaddr, cpath, size, rodata, defsyms, keep, flags=flags)
            print("$ " + oracle_cmdline(argv))
            rc, facts, text, err = run_oracle(argv)
    elif facts and facts.get("unresolved_symbols"):
        other = list(facts["unresolved_symbols"])

    result = facts["result"] if facts else "ERROR"
    masked = facts.get("masked_words", 0) if facts else 0
    ours = facts.get("bytes") if facts else None
    first = facts.get("first_diff_offset") if facts else None
    dwords = facts.get("diff_words") if facts else None
    clean = rc == 0 and result == "MATCH" and masked == 0 and (size is None or ours == size)
    tag = result + ("(masked)" if result == "MATCH" and masked else "")
    attempts = append_sidecar(sidecar_path(re_dir, unit, vaddr_s), tag, first, dwords)

    print(text.rstrip("\n"))
    if err.strip():
        print(err.rstrip("\n"), file=sys.stderr)
    print("")
    print("result: %s   size ours/expected: %s/%s   first diff: %s   attempts: %d"
          % (tag, ours if ours is not None else "?", size if size is not None else "?",
             "-" if first is None else "0x%x" % first, attempts))
    if other:
        print("unresolved: %s - needs --defsym NAME=0xADDR or a markup name" % ", ".join(other))
    hint = pick_hint(rc, facts, text, size)
    if hint and not clean:
        print("hint: " + hint)
    if keep:
        print("kept work dir: " + keep)

    if not clean:
        return 2 if result == "ERROR" else 1

    if scratch:
        print("clean MATCH in scratch dir %s: manifest/ledger not updated" % rel(re_dir))
        return 0
    extra = []
    if "--base" in u.oracle_flags:
        extra += ["--base", u.oracle_flags[u.oracle_flags.index("--base") + 1]]
    if rodata:
        extra += ["--rodata", rodata]
    extra += flags
    syms = syms_args(cpath)
    extra += syms
    for d in (drop_table_defsyms(defsyms, table_syms()) if syms else defsyms):
        extra += ["--defsym", d]
    src = "%s/%s.c" % (unit, fn_name(vaddr_s))
    added = manifest_append(vaddr_s, u.manifest_unit, src, ours, " ".join(extra))
    print("manifest: %s" % ("row added" if added else "row already present (not duplicated)"))
    lpath = ledger.ledger_path()
    if ledger.state(unit, vaddr_s) == "match":
        print("ledger: already a match in %s (no row added)" % rel(lpath))
    else:
        ledger.record("match", unit, vaddr_s, attempts)
        print("ledger: match recorded in %s" % rel(lpath))
    files = [rel(cpath), rel(MANIFEST)]
    if not rel(lpath).startswith("/") and ":" not in rel(lpath):
        files.append(rel(lpath))
    msg = "matched %s %d" % (fn_name(vaddr_s), attempts)
    cmdline = "git add %s && git commit -m %s -m %s" % (" ".join(files), shlex.quote(msg),
                                                        shlex.quote(COMMIT_TRAILER))
    print("commit: " + cmdline)
    if args.commit:
        subprocess.run(["git", "add"] + files, cwd=REPO, check=True)
        subprocess.run(["git", "commit", "-q", "-m", msg, "-m", COMMIT_TRAILER], cwd=REPO, check=True)
        print("committed.")
    return 0


# ---------------------------------------------------------------------------
# giveup / status / verify
# ---------------------------------------------------------------------------
def cmd_giveup(args):
    unit = ledger.norm_unit(args.unit)
    vaddr_s = ledger.norm_vaddr(args.vaddr)
    re_dir = os.path.abspath(args.re_dir)
    reason = " ".join(args.reason).strip() or "no reason given"
    n = len(read_sidecar(sidecar_path(re_dir, unit, vaddr_s)))
    r = ledger.record("giveup", unit, vaddr_s, n, reason)
    print("ledger: giveup %s %s attempts=%d: %s" % (r["unit"], r["vaddr"], n, r["reason"]))
    cpath = c_path(re_dir, unit, vaddr_s)
    if os.path.isfile(cpath):
        if is_given_up(cpath):
            print("%s already carries a %s header" % (rel(cpath), GIVEUP_MARK))
        else:
            with open(cpath, encoding="utf-8", errors="replace") as fh:
                body = fh.read()
            with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
                fh.write("/* %s: %s, %d attempts */\n%s" % (GIVEUP_MARK, reason.replace("*/", "* /"), n, body))
            print("%s: header added" % rel(cpath))
    else:
        print("no C file at %s" % rel(cpath))
    return 0


def cmd_status(args):
    re_dir = os.path.abspath(args.re_dir)
    current = ledger.load()
    if args.queue:
        rows = read_queue(args.queue)
    else:
        allrows = load_rows()
        rows = []
        for (unit, vaddr), e in sorted(current.items()):
            r = allrows.get((unit, vaddr)) or {"unit": unit, "vaddr": vaddr, "name": "?", "score": 0.0}
            rows.append(r)
        for (unit, vaddr), r in sorted(allrows.items()):
            if (unit, vaddr) not in current and os.path.isfile(sidecar_path(re_dir, unit, vaddr)):
                rows.append(r)
    print("%-7s %-10s %-16s %6s %-7s %8s  %s" % ("unit", "vaddr", "name", "score", "state", "attempts", "last"))
    tot = {"open": 0, "match": 0, "giveup": 0}
    tries = 0
    for r in rows:
        st = ledger.state(r["unit"], r["vaddr"], current)
        side = read_sidecar(sidecar_path(re_dir, r["unit"], r["vaddr"]))
        if st == "open" and is_given_up(c_path(re_dir, r["unit"], r["vaddr"])):
            st = "giveup*"      # header in the C, not (yet) in this ledger
        last = side[-1].split("\t") if side else []
        last_s = "%s %s" % (last[1], "" if len(last) < 3 or last[2] == "-" else "@" + last[2]) if last else "-"
        n = len(side) or current.get((r["unit"], r["vaddr"]), {}).get("attempts", 0)
        tries += len(side)
        tot[st.rstrip("*")] += 1
        print("%-7s %-10s %-16s %6.3f %-7s %8d  %s" % (r["unit"], r["vaddr"], r.get("name", "?"),
                                                       float(r.get("score", 0)), st, n, last_s.strip()))
    print("total: %d function(s): %d matched, %d given up, %d open; %d try line(s) in sidecars; ledger %s"
          % (len(rows), tot["match"], tot["giveup"], tot["open"], tries, rel(ledger.ledger_path())))
    return 0


def verify_row(r, base):
    """-> (ok?, final line) for one manifest row."""
    try:
        u = units.resolve(r["unit"] or "exe")
    except (units.UnitError, ValueError) as e:
        return False, "ERROR: %s" % e
    extra = shlex.split(r["extra"]) if r["extra"] else []
    if "--base" in u.oracle_flags and "--base" in extra:
        i = extra.index("--base")
        del extra[i:i + 2]
    argv = [r["vaddr"], os.path.join(base, r["src"])] + list(u.oracle_flags)
    if r["size"]:
        argv += ["--expect-size", r["size"]]
    argv += extra + ["-q"]
    res = subprocess.run([sys.executable, MATCH_FN] + argv, cwd=REPO,
                         capture_output=True, text=True, errors="replace")
    lines = [ln for ln in res.stdout.splitlines() if ln.strip()]
    line = lines[-1] if lines else ("(no output) " + res.stderr.strip()[-200:])
    return res.returncode == 0 and line.startswith("MATCH"), line


def cmd_verify(args):
    import concurrent.futures as cf
    path = os.path.abspath(args.manifest)
    _header, rows, _crlf = read_manifest(path)
    base = os.path.dirname(path)
    n = fail = 0
    # the oracle is one short process per row: threads only wait on it, so -j scales with cores;
    # results are printed in manifest order
    with cf.ThreadPoolExecutor(max_workers=max(1, args.jobs)) as ex:
        for r, (ok, line) in zip(rows, ex.map(lambda r: verify_row(r, base), rows)):
            n += 1
            if ok:
                if not args.quiet:
                    print("ok    %s  %s" % (r["vaddr"], line))
            else:
                print("FAIL  %s  %s  (%s %s)" % (r["vaddr"], line, r["unit"], r["src"]))
                fail += 1
    print("%d/%d matched" % (n - fail, n))
    return 0 if fail == 0 and n > 0 else 1


# ---------------------------------------------------------------------------
def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    if argv[:1] == ["--replay-twins"]:          # implemented in twins.py (see README)
        import twins
        return twins.main(["replay"] + list(argv[1:]))
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd")

    p = sub.add_parser("next", help="print the next work package (exit 3 when exhausted)")
    p.add_argument("--unit")
    p.add_argument("--queue", help="matcher queue TSV (needs $AC1_LEDGER)")
    p.add_argument("--shard", help="k/n: keep functions with (vaddr//4) %% n == k")
    p.add_argument("--max-score", type=float)
    p.add_argument("--no-m2c", action="store_true")
    p.add_argument("--dry-run", action="store_true", help="do not write the skeleton")
    p.add_argument("--re-dir", default=RE_MATCH, help="where the C files live (default re/match)")
    p.set_defaults(func=cmd_next)

    p = sub.add_parser("try", help="run the oracle on re/match/<unit>/func_<ADDR>.c")
    p.add_argument("unit")
    p.add_argument("vaddr")
    p.add_argument("--defsym", action="append", metavar="NAME=ADDR")
    p.add_argument("--rodata", metavar="VA")
    p.add_argument("--no-rodata-align-fix", action="store_true",
                   help="keep cc1's `.align 3` before jump tables (match_fn.py flag)")
    p.add_argument("--no-auto-defsym", action="store_true")
    p.add_argument("--commit", action="store_true", help="run the commit command on a clean MATCH")
    p.add_argument("--keep", action="store_true", help="keep the oracle work dir (printed)")
    p.add_argument("--re-dir", default=RE_MATCH)
    p.set_defaults(func=cmd_try)

    p = sub.add_parser("giveup", help="record a give-up and mark the C file")
    p.add_argument("unit")
    p.add_argument("vaddr")
    p.add_argument("reason", nargs="*")
    p.add_argument("--re-dir", default=RE_MATCH)
    p.set_defaults(func=cmd_giveup)

    p = sub.add_parser("status", help="state / attempts / last result per function")
    p.add_argument("--queue")
    p.add_argument("--re-dir", default=RE_MATCH)
    p.set_defaults(func=cmd_status)

    p = sub.add_parser("verify", help="re-run the oracle on every manifest row")
    p.add_argument("--manifest", default=MANIFEST)
    p.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4,
                   help="parallel oracle runs (default: CPU count)")
    p.add_argument("-q", "--quiet", action="store_true", help="print only FAIL rows and the total")
    p.set_defaults(func=cmd_verify)

    args = ap.parse_args(argv)
    if not args.cmd:
        ap.print_help()
        return 2
    try:
        return args.func(args)
    except ValueError as e:
        die(str(e))


if __name__ == "__main__":
    sys.exit(main())
