#!/usr/bin/env python3
"""divergence.py - twin bodies that differ between ovl202, ovl203 and ovl204.

    python tools/archaeology/divergence.py [--min-size 0] [--detail N]
                                           [--out tools/archaeology/twin_divergence.tsv]

The three in-game program overlays are one source tree linked three ways
(same module order).  This script aligns their function lists in link order
and reports every aligned pair whose relocation-masked body differs:

1. Alignment.  Each function is keyed by its relocation-masked body
   (twins.mask_function).  ovl203 and ovl204 are each aligned to ovl202 with
   difflib on the key sequence; inside every replaced block, functions are
   paired greedily by coarse-skeleton similarity (all immediates masked),
   ratio >= 0.5.  Unpaired functions are copy-specific (inserted / dropped).
2. Diff.  For a paired function with a different masked body, the word
   streams are diffed (difflib on the coarse skeleton, then on exact masked
   words) and the change is classified:
      size        instruction count changed
      frame       addiu sp,sp,-N differs
      const       same opcodes, different non-relocation immediates
                  (struct offsets, li constants, shift amounts)
      code        opcodes inserted / deleted / replaced
   with the counts of inserted / deleted words and the first differing
   instructions (as byte offsets into each copy; disassemble them with objdump).
3. Context.  The closest preceding doc-labelled function of ovl202 (from
   doc_labels.tsv) is printed so each divergence can be read in its system.

Output TSV: one row per divergent pair or copy-specific function.
"""
import argparse
import bisect
import difflib
import os
import sys
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import arch  # noqa: E402
import twins  # noqa: E402

OUT = os.path.join(HERE, "twin_divergence.tsv")
LABELS = os.path.join(HERE, "doc_labels.tsv")


def coarse(words):
    return tuple(w & 0xFC000000 if (w >> 26) in (2, 3) else w if (w >> 26) == 0
                 else w & 0xFFFF0000 for w in words)


def load(unit):
    rows = sorted(arch.load_rows([unit]), key=lambda r: r["va"])
    for r in rows:
        r["w"] = twins.func_words(unit, r["va"], r["sz"])
        r["m"], _ = twins.mask_function(r["w"], r["va"])
        r["c"] = coarse(r["w"])
    return rows


def labels():
    out = []
    if os.path.exists(LABELS):
        with open(LABELS, encoding="utf-8") as fh:
            for line in fh:
                if line.startswith("#") or line.startswith("unit\t"):
                    continue
                p = line.rstrip("\n").split("\t")
                if len(p) >= 5 and p[0] == "ovl202":
                    out.append((int(p[1], 16), p[2], p[3]))
    out.sort()
    return out


def label_for(lab, va):
    k = bisect.bisect_right([x[0] for x in lab], va) - 1
    return "%s (%s @0x%08X)" % (lab[k][2], lab[k][1], lab[k][0]) if k >= 0 else ""


def align(a, b):
    """[(ra or None, rb or None)] in link order."""
    sm = difflib.SequenceMatcher(None, [r["m"] for r in a], [r["m"] for r in b], autojunk=False)
    out = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            out.extend(zip(a[i1:i2], b[j1:j2]))
            continue
        A, B = a[i1:i2], b[j1:j2]
        used = set()
        pairs = {}
        cand = []
        for x, ra in enumerate(A):
            for y, rb in enumerate(B):
                s = difflib.SequenceMatcher(None, ra["c"], rb["c"], autojunk=False)
                if s.real_quick_ratio() < 0.5 or s.quick_ratio() < 0.5:
                    continue
                r = s.ratio()
                if r >= 0.5:
                    cand.append((r, -abs(x - y), x, y))
        for r, _, x, y in sorted(cand, reverse=True):
            if x in pairs or y in used:
                continue
            pairs[x] = y
            used.add(y)
        y0 = 0
        for x, ra in enumerate(A):
            if x in pairs:
                for y in range(y0, pairs[x]):
                    if y not in used:
                        out.append((None, B[y]))
                out.append((ra, B[pairs[x]]))
                y0 = max(y0, pairs[x] + 1)
            else:
                out.append((ra, None))
        for y in range(y0, len(B)):
            if y not in used:
                out.append((None, B[y]))
    return out


def frame(words):
    for w in words[:12]:
        if (w >> 16) == 0x27BD and w & 0x8000:
            return 0x10000 - (w & 0xFFFF)
    return 0


def classify(ra, rb):
    kinds = []
    if len(ra["w"]) != len(rb["w"]):
        kinds.append("size")
    if frame(ra["w"]) != frame(rb["w"]):
        kinds.append("frame")
    sm = difflib.SequenceMatcher(None, ra["c"], rb["c"], autojunk=False)
    ins = dele = rep = 0
    first = None
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            continue
        if first is None:
            first = (i1, j1)
        if tag == "insert":
            ins += j2 - j1
        elif tag == "delete":
            dele += i2 - i1
        else:
            rep += max(i2 - i1, j2 - j1)
    const = 0
    if not (ins or dele or rep):
        for i, (x, y) in enumerate(zip(ra["m"], rb["m"])):
            if x != y:
                const += 1
                if first is None:
                    first = (i, i)
    if ins or dele or rep:
        kinds.append("code")
    elif const:
        kinds.append("const")
    return kinds, ins, dele, rep, const, first


def triples(base, lab, detail):
    """3-way pattern per ovl202 function: which of the copies agree."""
    b3, b4 = load("ovl203"), load("ovl204")
    m3 = {id(x): y for x, y in align(base, b3) if x and y}
    m4 = {id(x): y for x, y in align(base, b4) if x and y}
    pat = Counter()
    late = []
    for x in base:
        y, z = m3.get(id(x)), m4.get(id(x))
        if y is None or z is None:
            pat["missing in %s" % ("203+204" if y is None and z is None
                                   else "203" if y is None else "204")] += 1
            continue
        k = (x["m"] == y["m"], x["m"] == z["m"], y["m"] == z["m"])
        name = {(True, True, True): "all equal",
                (False, False, True): "202 differs, 203 == 204 (mission vs versus build)",
                (True, False, False): "204 differs (202 == 203)",
                (False, True, False): "203 differs (202 == 204)",
                (False, False, False): "all three differ"}[k]
        pat[name] += 1
        if not k[2]:
            kinds, ins, dele, rep, const, first = classify(y, z)
            late.append((x, y, z, name, kinds, ins, dele, rep, const))
    print("3-way patterns over ovl202's functions:", file=sys.stderr)
    for k, v in pat.most_common():
        print("   %4d  %s" % (v, k), file=sys.stderr)
    print("203 vs 204 divergences (late-edit candidates), biggest first:", file=sys.stderr)
    late.sort(key=lambda t: -(t[5] + t[6] + t[7] + t[8]))
    for x, y, z, name, kinds, ins, dele, rep, const in late[:detail]:
        print("   202 0x%08X %5d | 203 0x%08X %5d | 204 0x%08X %5d  %-17s %-15s +%d -%d ~%d c%d  %s" % (
            x["va"], x["sz"], y["va"], y["sz"], z["va"], z["sz"], name.split(" (")[0],
            ",".join(kinds), ins, dele, rep, const, label_for(lab, x["va"])), file=sys.stderr)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--min-size", type=int, default=0)
    ap.add_argument("--detail", type=int, default=40,
                    help="print this many 203-vs-204 divergences (the late-edit candidates)")
    ap.add_argument("--out", default=OUT)
    a = ap.parse_args(argv)
    lab = labels()
    base = load("ovl202")
    rows = []
    for other in ("ovl203", "ovl204"):
        al = align(base, load(other))
        same = sum(1 for x, y in al if x and y and x["m"] == y["m"])
        div = [(x, y) for x, y in al if x and y and x["m"] != y["m"]]
        only202 = [x for x, y in al if x and not y]
        onlyo = [y for x, y in al if y and not x]
        print("ovl202 vs %s: %d identical twins, %d divergent twins, %d only in 202 (%d B), "
              "%d only in %s (%d B)" % (other, same, len(div), len(only202),
                                         sum(r["sz"] for r in only202), len(onlyo), other,
                                         sum(r["sz"] for r in onlyo)), file=sys.stderr)
        for x, y in div:
            if max(x["sz"], y["sz"]) < a.min_size:
                continue
            kinds, ins, dele, rep, const, first = classify(x, y)
            fa = ""
            if first:
                i, j = first
                fa = "+0x%X/+0x%X" % (4 * i, 4 * j)   # offsets only: no game bytes in the TSV
            rows.append((other, "diverged", x["va"], x["sz"], y["va"], y["sz"], ",".join(kinds),
                         ins, dele, rep, const, fa, label_for(lab, x["va"])))
        for x in only202:
            rows.append((other, "only202", x["va"], x["sz"], 0, 0, "", 0, 0, 0, 0, "",
                         label_for(lab, x["va"])))
        for y in onlyo:
            rows.append((other, "only_" + other, 0, 0, y["va"], y["sz"], "", 0, 0, 0, 0, "", ""))
    triples(base, lab, a.detail)
    with open(a.out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("vs\tstatus\tva202\tsize202\tva_other\tsize_other\tkinds\tins\tdel\trep\tconst"
                 "\tfirst_diff\tnearest_202_label\n")
        for r in rows:
            fh.write("%s\t%s\t0x%08X\t%d\t0x%08X\t%d\t%s\t%d\t%d\t%d\t%d\t%s\t%s\n" % r)
    print("wrote %s (%d rows)" % (a.out, len(rows)), file=sys.stderr)


if __name__ == "__main__":
    main()
