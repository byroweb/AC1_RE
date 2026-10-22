#!/usr/bin/env python3
"""hints.py - human insight about open functions: re/match/hints.tsv.

    hints.py show [UNIT VADDR]          hints for one function (or all), newest last
    hints.py add UNIT VADDR KIND TEXT   append one row (KIND: hint, name, field, triage, steer)
    hints.py names [--apply NAME ...]   proposed names as include/ac1/symbols.tsv rows

re/match/hints.tsv is append-only, one row per note, columns
`unit vaddr kind text timestamp`:

  hint    free text ("draws the radar", "loops over the 8 AC parts")
  name    a proposed name for the function: `radar_draw`, or a whole prototype
          `void radar_draw(UiObj *o)` (gives the symbols.tsv C type too)
  field   a proposed struct field name, free form (`AcEntity +0x4C hp`)
  triage  priority | skip | giveup | none; the newest triage row of a function wins
  steer   deep (give it the overnight deep pass even if it is not among the closest)
          | agent (hand it to an agent lane / the local-LLM golem; the viewer also
          writes a priority row) | none

browser-tools/match-viewer/viewer.py writes it (POST /api/hints); make_queues.py puts priority bodies
first and drops skip / giveup ones; the match-function skill tells agents to read a
function's hints before the first draft; a twin's hint counts for the whole body.

`names` never edits anything by itself: it prints the symbols.tsv row each name
proposal would become, with what blocks it (no C type yet, name or address taken).
`--apply NAME ...` appends the chosen rows, runs `ctx.py gen` and `ctx.py check`, and
puts symbols.tsv / symbols.h back if check fails.  Field proposals are listed for a
person to put into include/ac1/*.h by hand.
"""
import argparse
import datetime
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
HINTS = os.path.join(REPO, "re", "match", "hints.tsv")
COLS = ["unit", "vaddr", "kind", "text", "timestamp"]
KINDS = ("hint", "name", "field", "triage", "steer")
TRIAGE = ("priority", "skip", "giveup", "none")
STEER = ("deep", "agent", "none")
HEADER = "\t".join(COLS)
IDENT = re.compile(r"^[A-Za-z_]\w*$")
PROTO = re.compile(r"^(?P<ret>.*?[\w*)\]]\s*\**)\s*(?P<name>[A-Za-z_]\w*)\s*(?P<params>\(.*\))\s*;?$")


def norm_unit(u):
    u = str(u).strip().lower()
    if u in ("exe", "main", "slus"):
        return "exe"
    m = re.fullmatch(r"(?:ovl|fdat)?0*(\d+)", u)
    if not m:
        raise ValueError("bad unit %r (exe, ovl201..ovl204, fdatNNN)" % u)
    n = int(m.group(1))
    return "ovl%d" % n if 201 <= n <= 204 else "fdat%03d" % n


def norm_vaddr(v):
    v = str(v).strip().lower()
    if v.startswith("func_"):
        v = v[5:]
    return "0x%08X" % int(v[2:] if v.startswith("0x") else v, 16)


def clean_text(s):
    """One TSV cell: tabs and line breaks become spaces."""
    return " ".join(str(s).replace("\t", " ").split())


def now():
    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def load(path=None):
    """All rows in file order, as dicts with normalised unit/vaddr."""
    path = path or HINTS
    rows = []
    if not os.path.isfile(path):
        return rows
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\r\n")
            if not line.strip() or line.startswith("#") or line == HEADER:
                continue
            p = line.split("\t")
            if len(p) < 4:
                continue
            try:
                r = {"unit": norm_unit(p[0]), "vaddr": norm_vaddr(p[1]), "kind": p[2].strip(),
                     "text": p[3], "timestamp": p[4] if len(p) > 4 else ""}
            except ValueError:
                continue
            rows.append(r)
    return rows


def validate(kind, text):
    """-> cleaned text, or raises ValueError."""
    if kind not in KINDS:
        raise ValueError("kind must be one of %s" % ", ".join(KINDS))
    text = clean_text(text)
    if not text:
        raise ValueError("empty text")
    if kind == "triage" and text not in TRIAGE:
        raise ValueError("triage must be one of %s" % ", ".join(TRIAGE))
    if kind == "steer" and text not in STEER:
        raise ValueError("steer must be one of %s" % ", ".join(STEER))
    if kind == "name" and not (IDENT.match(text) or PROTO.match(text)):
        raise ValueError("a name is a C identifier or a prototype like `void f(UiObj *o)`")
    if len(text) > 2000:
        raise ValueError("text longer than 2000 characters")
    return text


def append(unit, vaddr, kind, text, path=None, timestamp=None):
    """Append one validated row; -> the row dict."""
    path = path or HINTS
    row = {"unit": norm_unit(unit), "vaddr": norm_vaddr(vaddr), "kind": kind,
           "text": validate(kind, text), "timestamp": timestamp or now()}
    new = not os.path.isfile(path) or os.path.getsize(path) == 0
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "a", encoding="utf-8", newline="\n") as fh:
        if new:
            fh.write(HEADER + "\n")
        fh.write("\t".join(row[c] for c in COLS) + "\n")
    return row


def delete(row, path=None):
    """Remove the first row equal to `row` (unit, vaddr, kind, text, timestamp). -> bool."""
    path = path or HINTS
    if not os.path.isfile(path):
        return False
    key = (norm_unit(row["unit"]), norm_vaddr(row["vaddr"]), row["kind"], row["text"], row["timestamp"])
    with open(path, encoding="utf-8") as fh:
        lines = fh.readlines()
    for i, line in enumerate(lines):
        p = line.rstrip("\r\n").split("\t")
        if len(p) >= 5 and p[2] == key[2] and p[3] == key[3] and p[4] == key[4]:
            try:
                if (norm_unit(p[0]), norm_vaddr(p[1])) != key[:2]:
                    continue
            except ValueError:
                continue
            del lines[i]
            with open(path, "w", encoding="utf-8", newline="\n") as fh:
                fh.writelines(lines)
            return True
    return False


def by_function(rows):
    """{(unit, vaddr): [rows]}."""
    out = {}
    for r in rows:
        out.setdefault((r["unit"], r["vaddr"]), []).append(r)
    return out


def triage(rows):
    """{(unit, vaddr): priority|skip|giveup} from the newest triage row of each function."""
    out = {}
    for r in rows:
        if r["kind"] == "triage":
            out[(r["unit"], r["vaddr"])] = r["text"]
    return {k: v for k, v in out.items() if v != "none"}


def steer(rows, what):
    """[(unit, vaddr)] whose newest steer row is `what` (deep / agent), in file order."""
    last = {}
    for r in rows:
        if r["kind"] == "steer":
            last[(r["unit"], r["vaddr"])] = r["text"]
    return [k for k, v in last.items() if v == what]


def for_function(unit, vaddr, rows=None):
    k = (norm_unit(unit), norm_vaddr(vaddr))
    return [r for r in (load() if rows is None else rows) if (r["unit"], r["vaddr"]) == k]


def body_triage(members, flags):
    """Triage of a twin body from its members' flags: skip/giveup on any member wins,
    then priority; None when no member is flagged."""
    got = {flags.get((norm_unit(u), norm_vaddr(v))) for u, v in members}
    for f in ("skip", "giveup", "priority"):
        if f in got:
            return f
    return None


# ---------------------------------------------------------------------------
# names -> symbols.tsv rows for review
# ---------------------------------------------------------------------------
def symbols_unit(unit):
    """hints unit -> symbols.tsv unit column (exe, 201..204, fdatNNN)."""
    return unit[3:] if unit.startswith("ovl") else unit


def matched_ctype(unit, vaddr):
    """`ret (params)` from the matched C file's definition, or None."""
    p = os.path.join(REPO, "re", "match", unit, "func_%s.c" % vaddr[2:])
    if not os.path.isfile(p):
        return None
    with open(p, encoding="utf-8", errors="replace") as fh:
        text = re.sub(r"/\*.*?\*/", " ", fh.read(), flags=re.S)
    m = re.search(r"^([^\n;#{}]*?)\bfunc_%s\s*(\([^)]*\))\s*\{" % vaddr[2:], text, re.M)
    if not m:
        return None
    ret = " ".join(m.group(1).replace("static", "").split()) or "int"
    return "%s %s" % (ret, " ".join(m.group(2).split()))


def table_names():
    """name -> address from symbols.tsv and ac1_markup.tsv (as drive.table_syms reads them)."""
    sys.path.insert(0, HERE)
    import drive
    return drive.table_syms()


def symbols_addrs():
    out = {}
    path = os.path.join(REPO, "include", "ac1", "symbols.tsv")
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            p = line.rstrip("\r\n").split("\t")
            if len(p) == 4 and re.fullmatch(r"0x[0-9A-Fa-f]{8}", p[0].strip()):
                out[(int(p[0], 16), p[3].strip())] = p[1].strip()
    return out


def proposals(rows, names=None, addrs=None):
    """[{unit, vaddr, name, ctype, row, problems}] for the newest name hint of each function."""
    names = table_names() if names is None else names
    addrs = symbols_addrs() if addrs is None else addrs
    newest = {}
    for r in rows:
        if r["kind"] == "name":
            newest[(r["unit"], r["vaddr"])] = r
    out = []
    for (unit, vaddr), r in sorted(newest.items()):
        text = r["text"].strip().rstrip(";")
        m = PROTO.match(text)
        if m and not IDENT.match(text):
            name, ctype = m.group("name"), "%s %s" % (" ".join(m.group("ret").split()), m.group("params"))
        else:
            name, ctype = text, matched_ctype(unit, vaddr)
        problems = []
        if not ctype:
            problems.append("no C type: give the hint as a prototype, or match the function first")
        va = int(vaddr, 16)
        if name in names and names[name] != va:
            problems.append("%s is already 0x%08X" % (name, names[name]))
        su = symbols_unit(unit)
        if (va, su) in addrs:
            problems.append("0x%08X already named %s in symbols.tsv" % (va, addrs[(va, su)]))
        out.append({"unit": unit, "vaddr": vaddr, "name": name, "ctype": ctype or "?",
                    "row": "0x%08X\t%s\t%s\t%s" % (va, name, ctype or "?", su), "problems": problems,
                    "hint": r})
    return out


def apply_rows(rows):
    """Append rows to symbols.tsv, ctx.py gen + check; restore both files on failure."""
    tsv = os.path.join(REPO, "include", "ac1", "symbols.tsv")
    h = os.path.join(REPO, "include", "ac1", "symbols.h")
    saved = {}
    for p in (tsv, h):
        with open(p, "rb") as fh:
            saved[p] = fh.read()
    with open(tsv, "ab") as fh:
        body = saved[tsv]
        if body and not body.endswith(b"\n"):
            fh.write(b"\n")
        fh.write(("\n".join(rows) + "\n").encode("utf-8"))
    ctx = os.path.join(REPO, "tools", "match", "ctx.py")
    for cmd in ("gen", "check"):
        r = subprocess.run([sys.executable, ctx, cmd], cwd=REPO, capture_output=True, text=True,
                           errors="replace")
        sys.stdout.write(r.stdout)
        sys.stderr.write(r.stderr)
        if r.returncode:
            for p, b in saved.items():
                with open(p, "wb") as fh:
                    fh.write(b)
            print("ctx.py %s failed: symbols.tsv and symbols.h restored" % cmd)
            return 1
    print("applied %d row(s); review `git diff include/ac1` and commit symbols.tsv + symbols.h "
          "on their own" % len(rows))
    return 0


def cmd_names(args):
    rows = load(args.hints)
    props = proposals(rows)
    fields = [r for r in rows if r["kind"] == "field"]
    if not props and not fields:
        print("no name or field proposals in %s" % os.path.relpath(args.hints, REPO))
        return 0
    if props:
        print("# proposed include/ac1/symbols.tsv rows (address, name, C type, unit)")
        for p in props:
            print(p["row"] + ("" if not p["problems"] else "\t# BLOCKED: " + "; ".join(p["problems"])))
    if fields:
        print("\n# struct field proposals (edit include/ac1/*.h by hand, then ctx.py check)")
        for r in fields:
            print("%s %s  %s" % (r["unit"], r["vaddr"], r["text"]))
    if not args.apply:
        print("\n# nothing applied: hints.py names --apply NAME ... appends the chosen rows, "
              "then runs ctx.py gen + check")
        return 0
    want = set(args.apply)
    pick = [p for p in props if p["name"] in want]
    missing = want - {p["name"] for p in pick}
    if missing:
        print("not proposed: %s" % ", ".join(sorted(missing)))
        return 2
    blocked = [p for p in pick if p["problems"]]
    if blocked:
        print("blocked, not applied: %s" % ", ".join(p["name"] for p in blocked))
        return 2
    return apply_rows([p["row"] for p in pick])


def cmd_show(args):
    rows = load(args.hints)
    if args.unit:
        rows = for_function(args.unit, args.vaddr, rows)
    for r in rows:
        print("%s %s  %-6s %s  (%s)" % (r["unit"], r["vaddr"], r["kind"], r["text"], r["timestamp"]))
    if not rows:
        print("no hints")
    return 0


def cmd_add(args):
    r = append(args.unit, args.vaddr, args.kind, " ".join(args.text), args.hints)
    print("\t".join(r[c] for c in COLS))
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--hints", default=HINTS, help="hints file (default re/match/hints.tsv)")
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("show")
    p.add_argument("unit", nargs="?")
    p.add_argument("vaddr", nargs="?")
    p = sub.add_parser("add")
    p.add_argument("unit")
    p.add_argument("vaddr")
    p.add_argument("kind", choices=KINDS)
    p.add_argument("text", nargs="+")
    p = sub.add_parser("names")
    p.add_argument("--apply", nargs="+", metavar="NAME", help="append these proposals to symbols.tsv")
    args = ap.parse_args(argv)
    if args.cmd == "show" and args.unit and not args.vaddr:
        ap.error("show wants UNIT and VADDR")
    try:
        return {"show": cmd_show, "add": cmd_add, "names": cmd_names}[args.cmd](args)
    except ValueError as e:
        print("hints.py: %s" % e, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
