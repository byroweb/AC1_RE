#!/usr/bin/env python3
"""merge_manifest.py - clean up re/match/manifest.tsv after a merge.

    merge_manifest.py [MANIFEST] [--check]

Reads the manifest even when it still carries git conflict markers
(<<<<<<< / ======= / >>>>>>>: both sides are kept) or duplicate rows, takes
the union, dedupes on (vaddr, unit) (the first occurrence wins), sorts by
(unit, vaddr) with units ordered exe, 201..204, fdatNNN, and rewrites the
file with LF endings and the `#` header line preserved.  --check only reports
what would change (exit 1 if anything would).
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
DEFAULT = os.path.join(REPO, "re", "match", "manifest.tsv")
HEADER = ("# vaddr\tovl\tsource\tsize\textra match_fn.py args (one line per matched function; "
          "re/match/check_all.sh re-verifies every row)")
MARKER = re.compile(r"^(<{7}|={7}|>{7}|\|{7})( |$)")


def unit_key(u):
    u = (u or "exe").strip().lower()
    if u in ("exe", "", "main"):
        return (0, 0)
    m = re.fullmatch(r"(?:ovl)?(20[1-4])", u)
    if m:
        return (1, int(m.group(1)))
    m = re.fullmatch(r"(?:fdat)?0*(\d+)", u)
    if m:
        return (2, int(m.group(1)))
    return (3, u)


def norm_vaddr(v):
    v = v.strip().lower()
    if v.startswith("0x"):
        v = v[2:]
    try:
        return "0x%08X" % int(v, 16)
    except ValueError:
        return v


def parse(text):
    header = None
    rows = []
    for raw in text.splitlines():
        line = raw.rstrip("\r")
        if not line.strip() or MARKER.match(line):
            continue
        if line.startswith("#"):
            if header is None:
                header = line
            continue
        parts = line.split("\t")
        parts = [p.strip() for p in parts] + [""] * (5 - len(parts))
        if len(parts) > 5:
            parts = parts[:4] + ["\t".join(parts[4:]).strip()]
        rows.append(parts)
    return header, rows


def merge(rows):
    seen = set()
    out = []
    for r in rows:
        key = (norm_vaddr(r[0]), (r[1] or "exe").strip().lower())
        if key in seen:
            continue
        seen.add(key)
        out.append([norm_vaddr(r[0])] + r[1:])
    out.sort(key=lambda r: (unit_key(r[1]), int(r[0], 16) if r[0].startswith("0x") else 0))
    return out


def main(argv):
    check = "--check" in argv
    argv = [a for a in argv if a != "--check"]
    path = os.path.abspath(argv[0]) if argv else DEFAULT
    if not os.path.isfile(path):
        sys.exit("merge_manifest.py: %s not found" % path)
    with open(path, "rb") as fh:
        raw = fh.read().decode("utf-8", errors="replace")
    header, rows = parse(raw)
    merged = merge(rows)
    new = (header or HEADER) + "\n" + "".join("\t".join(r) + "\n" for r in merged)
    dropped = len(rows) - len(merged)
    changed = new != raw
    print("%d row(s) read, %d kept, %d duplicate(s) dropped%s"
          % (len(rows), len(merged), dropped,
             ", conflict markers removed" if any(MARKER.match(l) for l in raw.splitlines()) else ""))
    if check:
        print("would %s %s" % ("rewrite" if changed else "leave", os.path.relpath(path, os.getcwd())))
        return 1 if changed else 0
    if changed:
        with open(path, "wb") as fh:
            fh.write(new.encode("utf-8"))
        print("rewrote %s (LF endings)" % os.path.relpath(path, os.getcwd()))
    else:
        print("%s already clean" % os.path.relpath(path, os.getcwd()))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
