#!/usr/bin/env python3
"""ledger.py - append-only record of matching-loop outcomes (tools/loop/ledger.tsv).

One row per attempt outcome; the newest row for a (unit, vaddr) wins, so a
give-up can later be superseded by a match.  Never edit rows in place: append.

    ledger.py record-match  <unit> <vaddr> <attempts>
    ledger.py record-giveup <unit> <vaddr> <attempts> <reason...>
    ledger.py status                       # counts per unit (+ attempt stats)
    ledger.py is-matched <unit> <vaddr>    # exit 0 if matched, 1 otherwise
    ledger.py state <unit> <vaddr>         # prints match | giveup | open
    ledger.py import-manifest [re/match/manifest.tsv]
                                           # seed matches from the oracle's manifest;
                                           # attempts come from "matched func_X N" commits

<unit> is exe, ovl201..ovl204 or fdatNNN (a bare 201 is accepted as ovl201);
<vaddr> is hex with or without 0x.  Columns of ledger.tsv:

    when        ISO-8601 UTC timestamp
    unit        exe | ovl201 | ... | fdat000
    vaddr       0x-prefixed, 8 hex digits
    status      match | giveup
    attempts    oracle runs it took (0 = unknown)
    reason      free text for give-ups (tabs/newlines squashed), empty for matches

rank.py imports this module and skips every (unit, vaddr) whose latest status
is match or giveup.
"""
import datetime
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
DEFAULT_LEDGER = os.path.join(HERE, "ledger.tsv")
DEFAULT_MANIFEST = os.path.join(REPO, "re", "match", "manifest.tsv")
COLUMNS = ["when", "unit", "vaddr", "status", "attempts", "reason"]
PROGRAM_OVERLAYS = (201, 202, 203, 204)


def norm_unit(u):
    u = u.strip().lower()
    if u in ("exe", "main", "slus"):
        return "exe"
    m = re.fullmatch(r"(?:ovl|fdat)?0*(\d+)", u)
    if m:
        n = int(m.group(1))
        return "ovl%d" % n if n in PROGRAM_OVERLAYS else "fdat%03d" % n
    raise ValueError("bad unit %r (exe, ovl201..ovl204, fdatNNN)" % u)


def norm_vaddr(v):
    v = v.strip().lower()
    if v.startswith("func_"):
        v = v[5:]
    if v.startswith("0x"):
        v = v[2:]
    return "0x%08X" % int(v, 16)


def ledger_path():
    return os.environ.get("AC1_LEDGER", DEFAULT_LEDGER)


def load(path=None):
    """{(unit, vaddr): row dict} with the newest row per key."""
    path = path or ledger_path()
    out = {}
    if not os.path.isfile(path):
        return out
    with open(path, encoding="utf-8") as fh:
        header = None
        for line in fh:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if header is None:
                header = parts
                continue
            row = dict(zip(header, parts))
            try:
                row["attempts"] = int(row.get("attempts") or 0)
            except ValueError:
                row["attempts"] = 0
            out[(row["unit"], row["vaddr"])] = row
    return out


def append(row, path=None):
    path = path or ledger_path()
    new = not os.path.isfile(path) or os.path.getsize(path) == 0
    with open(path, "a", encoding="utf-8", newline="\n") as fh:
        if new:
            fh.write("\t".join(COLUMNS) + "\n")
        fh.write("\t".join(str(row.get(c, "")) for c in COLUMNS) + "\n")


def now():
    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def clean(text):
    return re.sub(r"\s+", " ", text).strip()


def record(status, unit, vaddr, attempts, reason="", path=None):
    row = {"when": now(), "unit": norm_unit(unit), "vaddr": norm_vaddr(vaddr),
           "status": status, "attempts": int(attempts), "reason": clean(reason)}
    append(row, path)
    return row


def state(unit, vaddr, rows=None):
    rows = load() if rows is None else rows
    r = rows.get((norm_unit(unit), norm_vaddr(vaddr)))
    return r["status"] if r else "open"


# ---------------------------------------------------------------------------
# commands
# ---------------------------------------------------------------------------
def cmd_status(args):
    rows = load()
    if not rows:
        print("ledger empty (%s)" % ledger_path())
        return 0
    per = {}
    for (unit, _), r in rows.items():
        d = per.setdefault(unit, {"match": 0, "giveup": 0, "att_m": [], "att_g": []})
        d[r["status"]] = d.get(r["status"], 0) + 1
        if r["attempts"]:
            d["att_m" if r["status"] == "match" else "att_g"].append(r["attempts"])
    print("%-8s %7s %7s %12s %12s" % ("unit", "matched", "giveup", "avg_att_m", "avg_att_g"))
    tm = tg = 0
    for unit in sorted(per):
        d = per[unit]
        am = "%.1f" % (sum(d["att_m"]) / len(d["att_m"])) if d["att_m"] else "-"
        ag = "%.1f" % (sum(d["att_g"]) / len(d["att_g"])) if d["att_g"] else "-"
        print("%-8s %7d %7d %12s %12s" % (unit, d["match"], d["giveup"], am, ag))
        tm += d["match"]
        tg += d["giveup"]
    print("%-8s %7d %7d" % ("total", tm, tg))
    return 0


def git_attempts():
    """{func_ADDR: attempts} from commit subjects 'matched func_<ADDR> <n>'."""
    out = {}
    try:
        log = subprocess.run(["git", "log", "--format=%s"], cwd=REPO,
                             capture_output=True, text=True, check=True).stdout
    except (subprocess.CalledProcessError, OSError):
        return out
    for line in log.splitlines():
        m = re.match(r"matched\s+func_([0-9A-Fa-f]{8})\s+(\d+)", line.strip())
        if m:
            out.setdefault(m.group(1).upper(), int(m.group(2)))  # newest first wins
    return out


def cmd_import_manifest(args):
    path = args[0] if args else DEFAULT_MANIFEST
    if not os.path.isfile(path):
        sys.exit("ledger.py: manifest %s not found" % path)
    rows = load()
    attempts = git_attempts()
    added = 0
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 2:
                continue
            vaddr = norm_vaddr(parts[0])
            unit = norm_unit(parts[1]) if parts[1].strip() else "exe"
            if rows.get((unit, vaddr), {}).get("status") == "match":
                continue
            n = attempts.get(vaddr[2:], 0)
            record("match", unit, vaddr, n, "", None)
            added += 1
            print("recorded match %s %s attempts=%d" % (unit, vaddr, n))
    print("%d row(s) added from %s" % (added, os.path.relpath(path, os.getcwd())))
    return 0


def main(argv):
    if not argv or argv[0] in ("-h", "--help"):
        print(__doc__)
        return 0
    cmd, args = argv[0], argv[1:]
    try:
        if cmd == "record-match":
            if len(args) != 3:
                sys.exit("usage: ledger.py record-match <unit> <vaddr> <attempts>")
            r = record("match", args[0], args[1], args[2])
            print("recorded match %s %s attempts=%d" % (r["unit"], r["vaddr"], r["attempts"]))
            return 0
        if cmd == "record-giveup":
            if len(args) < 4:
                sys.exit("usage: ledger.py record-giveup <unit> <vaddr> <attempts> <reason...>")
            r = record("giveup", args[0], args[1], args[2], " ".join(args[3:]))
            print("recorded giveup %s %s attempts=%d: %s"
                  % (r["unit"], r["vaddr"], r["attempts"], r["reason"]))
            return 0
        if cmd == "status":
            return cmd_status(args)
        if cmd == "is-matched":
            if len(args) != 2:
                sys.exit("usage: ledger.py is-matched <unit> <vaddr>")
            return 0 if state(args[0], args[1]) == "match" else 1
        if cmd == "state":
            if len(args) != 2:
                sys.exit("usage: ledger.py state <unit> <vaddr>")
            print(state(args[0], args[1]))
            return 0
        if cmd == "import-manifest":
            return cmd_import_manifest(args)
    except ValueError as e:
        sys.exit("ledger.py: %s" % e)
    sys.exit("ledger.py: unknown command %r (see --help)" % cmd)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
