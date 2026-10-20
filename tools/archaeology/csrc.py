#!/usr/bin/env python3
"""csrc.py - source-level idioms in the matched C (re/match/manifest.tsv).

    python tools/archaeology/csrc.py [--with-sweep]

Matched C is the recovered original style: every construct in it was needed
(or at least allowed) for GCC 2.7.2 to emit the retail bytes.  This script
reads each distinct source file once (twin rows share a file), strips
comments and the struct/typedef boilerplate, and counts idioms in the
function bodies:

    loops     do-while, while, for, goto, labels
    branches  switch, ternary, else, early return
    types     s8/u8/s16/u16/s32/u32/int/short/char locals and casts
    forcing   register keyword, volatile, asm, unused arrays / dummy locals,
              comments that say the shape was forced for the match
    pointers  p++ / *p++ walks, -> member access, array indexing

Each file is attributed to its manifest unit and to the doc's habit cluster
(A = ovl201 outside 0x800866FC-0x8009A7E8, B = that band, C = everything
else).  m2c drafts committed by tools/loop/sweep.py are skipped by default:
they carry m2c's idioms, not the original programmer's.  Output: one table
per group (unit / cluster / mission library vs mission-specific).
"""
import argparse
import os
import re
import sys
from collections import Counter, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
MANIFEST = os.path.join(REPO, "re", "match", "manifest.tsv")
TWINS = os.path.join(REPO, "tools", "loop", "twins.tsv")

PATTERNS = {
    "do_while": r"\bdo\s*\{",
    "while": r"(?<!\})\s*\bwhile\s*\(",
    "for": r"\bfor\s*\(",
    "goto": r"\bgoto\b",
    "switch": r"\bswitch\s*\(",
    "ternary": r"\?[^:;]*:",
    "else": r"\belse\b",
    "return": r"\breturn\b",
    "register": r"\bregister\b",
    "volatile": r"\bvolatile\b",
    "asm": r"\b__asm__|\basm\s*\(",
    "s8": r"\bs8\b|\bsigned char\b",
    "u8": r"\bu8\b|\bunsigned char\b",
    "s16": r"\bs16\b|\bshort\b",
    "u16": r"\bu16\b|\bunsigned short\b",
    "s32": r"\bs32\b|\bint\b|\blong\b",
    "u32": r"\bu32\b|\bunsigned int\b",
    "cast_narrow": r"\(\s*(?:s8|u8|s16|u16|short|char|unsigned char|unsigned short)\s*\)",
    "ptr_walk": r"\w\+\+\s*[;)=]|\*\s*\w+\+\+",
    "arrow": r"->",
    "index": r"\w\[[^\]]+\]",
    "macro": r"^\s*#\s*define\b",
}
FORCED = re.compile(r"forc|retail|permut|fake|dummy|unused|needed for|to match|keeps? the", re.I)


def strip(src):
    """(body text without comments/typedef blocks, comment text)."""
    comments = " ".join(re.findall(r"/\*.*?\*/|//[^\n]*", src, re.S))
    s = re.sub(r"/\*.*?\*/|//[^\n]*", "", src, flags=re.S)
    s = re.sub(r"typedef\s+struct\s*\w*\s*\{.*?\}\s*\w+\s*;", "", s, flags=re.S)
    s = re.sub(r"^\s*(extern|#include)[^\n]*\n", "", s, flags=re.M)
    return s, comments


def unit_group(unit):
    u = unit.strip()
    if u in ("201", "ovl201"):
        return "ovl201"
    if u in ("202", "203", "204", "ovl202", "ovl203", "ovl204", "ovl"):
        return "ovl202-family"
    if u == "exe":
        return "exe"
    return "fdat"


# the doc's habit clusters (docs/CODE_ARCHAEOLOGY.md section 2), by address
B_BAND = (0x800866FC, 0x8009A7E8)


def doc_cluster(group, va):
    if group == "ovl201":
        return "B" if B_BAND[0] <= va < B_BAND[1] else "A"
    return "C"


def is_sweep(src):
    """True for m2c drafts committed by tools/loop/sweep.py (m2c style, not recovered style)."""
    return "m2c_compat" in src or "M2C_" in src


def fdat_spread():
    """fdat source file -> number of units that carry its body (from the manifest)."""
    n = Counter()
    with open(MANIFEST, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#"):
                continue
            p = line.rstrip("\n").split("\t")
            if len(p) >= 3 and p[1].startswith("fdat"):
                n[p[2]] += 1
    return n


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--with-sweep", action="store_true",
                    help="also count the m2c sweep drafts (excluded by default)")
    a = ap.parse_args(argv)
    swept = 0
    spread = fdat_spread()
    seen = set()
    groups = defaultdict(Counter)
    files = defaultdict(int)
    with open(MANIFEST, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#"):
                continue
            p = line.rstrip("\n").split("\t")
            if len(p) < 3 or p[2] in seen:
                continue
            seen.add(p[2])
            path = os.path.join(REPO, "re", "match", p[2])
            if not os.path.exists(path):
                continue
            with open(path, encoding="utf-8", errors="replace") as f:
                src = f.read()
            if is_sweep(src) and not a.with_sweep:
                swept += 1
                continue
            body, comments = strip(src)
            g = unit_group(p[1])
            keys = [g, "cluster " + doc_cluster(g, int(p[0], 16))]
            if g == "fdat":
                keys.append("fdat-library (>=10 twins)" if spread[p[2]] >= 10 else "fdat-specific (<10)")
            c = Counter()
            for k, pat in PATTERNS.items():
                c[k] = len(re.findall(pat, body, re.M))
            c["do_while"] = len(re.findall(PATTERNS["do_while"], body))
            c["while"] -= c["do_while"]
            c["forced_note"] = 1 if FORCED.search(comments) else 0
            c["lines"] = sum(1 for ln in body.splitlines() if ln.strip())
            for k in keys:
                groups[k].update(c)
                files[k] += 1
    if swept:
        print("(%d m2c sweep drafts skipped; --with-sweep counts them)" % swept)
    cols = ["lines", "do_while", "while", "for", "goto", "switch", "ternary", "else", "return",
            "register", "volatile", "asm", "s8", "u8", "s16", "u16", "s32", "u32",
            "cast_narrow", "ptr_walk", "arrow", "index", "macro", "forced_note"]
    print("%-26s %5s " % ("group", "files") + " ".join("%7s" % c[:7] for c in cols))
    for g in sorted(groups):
        c = groups[g]
        print("%-26s %5d " % (g, files[g]) + " ".join("%7d" % c[k] for k in cols))
    print("\nper 100 non-blank lines:")
    for g in sorted(groups):
        c = groups[g]
        n = max(c["lines"], 1) / 100.0
        print("%-26s " % g + " ".join("%7.2f" % (c[k] / n) for k in cols[1:-1])
              + "  forced-notes %d/%d files" % (c["forced_note"], files[g]))


if __name__ == "__main__":
    main()
