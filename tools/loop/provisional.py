#!/usr/bin/env python3
"""Provisional matches: byte matches whose C steers the compiler with asm constructs.

A match counts once the oracle says MATCH. Some C gets there by forcing the compiler's
hand, though, and is probably not how the original source was written:

  register T v asm("$N");              a variable pinned to one register
  __asm__ ("" : "=r"(p) : "0"(&G));    an empty asm statement: an opaque value,
  __asm__ volatile ("" ::: "memory");  a memory clobber, or a scheduling barrier

Such matches stay in the manifest (their bytes are verified like any other), but are
listed in re/match/provisional.tsv and shown apart by progress.py, so they can be
reworked later. A twin replayed from a provisional C file is provisional too.
Non-empty asm (the PsyQ GTE macros such as `gte_ldv0`) is SDK code and does not count,
nor is a gp base pin (`register u8 *gp_base asm("$28")`), which stands in for the
game's gp-relative small-data addressing.

  python tools/loop/provisional.py scan     rewrite re/match/provisional.tsv
  python tools/loop/provisional.py check    exit 1 if the file is out of date
  python tools/loop/provisional.py list     print the rows
"""
import argparse
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import merge_manifest  # noqa: E402
import units  # noqa: E402

MANIFEST = os.path.join(REPO, "re", "match", "manifest.tsv")
OUT = os.path.join(REPO, "re", "match", "provisional.tsv")
HEADER = ("# unit\tvaddr\tsource\treason   (matches whose C uses register asm pins or empty asm "
          "statements; tools/loop/provisional.py scan)")
CONSTRUCTS = [
    # a gp ($28) base pin stands in for the game's gp-relative small-data addressing: not counted
    ("register pin", re.compile(r"\bregister\b[^;{}]*\b(?:__asm__|asm)\s*\(\s*\"(?!\$28\"|\$gp\"|gp\")")),
    ("empty asm", re.compile(r"\b(?:__asm__|asm)\s*(?:__volatile__|volatile)?\s*\(\s*\"\"")),
]


def reasons(path):
    """'register pin x2, empty asm x1' for a C file, or '' when it has none."""
    try:
        text = open(path, encoding="utf-8", errors="replace").read()
    except OSError:
        return ""
    code = re.sub(r"/\*.*?\*/|//[^\n]*", " ", text, flags=re.S)
    found = []
    for name, rx in CONSTRUCTS:
        n = len(rx.findall(code))
        if n:
            found.append("%s x%d" % (name, n))
    return ", ".join(found)


def compute(manifest=MANIFEST):
    """Sorted rows (unit, vaddr, source, reason) for every manifest row whose source is provisional."""
    _h, rows = merge_manifest.parse(open(manifest, encoding="utf-8").read())
    cache, out = {}, []
    for r in rows:
        try:
            unit, va = units.norm(r[1] or "exe"), int(r[0], 16)
        except (ValueError, units.UnitError):
            continue
        src = r[2]
        if src not in cache:
            cache[src] = reasons(os.path.join(REPO, "re", "match", src))
        if cache[src]:
            out.append((unit, "0x%08X" % va, src, cache[src]))
    return sorted(set(out))


def render(rows):
    return "\n".join([HEADER] + ["\t".join(r) for r in rows]) + "\n"


def load(path=OUT):
    """{(unit, vaddr)} listed in provisional.tsv (empty when the file is missing)."""
    out = set()
    if os.path.isfile(path):
        for line in open(path, encoding="utf-8"):
            p = line.rstrip("\r\n").split("\t")
            if len(p) >= 2 and not line.startswith("#"):
                out.add((p[0], int(p[1], 16)))
    return out


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("cmd", choices=("scan", "check", "list"))
    p.add_argument("--manifest", default=MANIFEST)
    p.add_argument("--out", default=OUT)
    args = p.parse_args(argv)
    rows = compute(args.manifest)
    if args.cmd == "scan":
        with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(render(rows))
        print("%d provisional match(es) from %d C file(s) -> %s"
              % (len(rows), len({r[2] for r in rows}), os.path.relpath(args.out, REPO)))
        return 0
    if args.cmd == "check":
        cur = open(args.out, encoding="utf-8").read() if os.path.isfile(args.out) else ""
        if cur.replace("\r\n", "\n") != render(rows):
            print("re/match/provisional.tsv is out of date: run tools/loop/provisional.py scan")
            return 1
        print("provisional.tsv up to date (%d rows)" % len(rows))
        return 0
    for r in rows:
        print("%-8s %s  %-28s %s" % r)
    print("%d provisional" % len(rows))
    return 0


if __name__ == "__main__":
    sys.exit(main())
