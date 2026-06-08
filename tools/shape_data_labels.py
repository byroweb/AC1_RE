#!/usr/bin/env python3
"""Shape the Farsi DATA-screen labels to game glyph bytes and check slot budgets.

Uses the validated runtime shaper (farsi_runtime_shape). Prints, per label, the
overlay address, Persian text, shaped hex (+ terminator), and PASS/FAIL vs the
slot byte budget. Menu/label strings are 0x00-terminated; the MISSION REPORT row
group (Sorties/…) is '>' (0x3e)-terminated (draw_string path).
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import farsi_runtime_shape as frs


def shape(word):
    return frs.shape_runtime(frs._word_to_tokens(word))


# addr -> (english, persian, terminator_byte, slot_size_bytes)
LABELS = {
    0x8004AF88: ("SAVE DATA",      "ذخیره داده", 0x00, 12),
    0x8004AF94: ("LOAD DATA",      "خواندن داده", 0x00, 12),
    0x8004AFA0: ("SAVE EMBLEM",    "ذخیره نشان", 0x00, 12),
    0x8004AFAC: ("LOAD EMBLEM",    "خواندن نشان", 0x00, 12),
    0x8004AFB8: ("OPTIONS",        "تنظیمات",     0x00, 8),
    0x8004AFC0: ("CREDITS",        "اعتبار",      0x00, 8),
    0x8004AFC8: ("RANKING",        "رتبه",        0x00, 8),
    0x8004AFD0: ("MISSION REPORT", "گزارش مموریت", 0x00, 16),
    0x8004AFE0: ("Sorties",        "مموریتها",    0x3e, 12),
    0x8004AFEC: ("Success",        "موفقیت",      0x3e, 12),
    0x8004AFF8: ("Failure",        "شکست",        0x3e, 12),
    0x8004B004: ("Overall",        "مجموع",       0x3e, 12),
}


def main():
    allok = True
    print(f"{'addr':<12}{'english':<16}{'persian':<14}{'glyphs':>6} {'total':>5} {'fit':>4}  hex")
    for addr, (en, fa, term, slot) in LABELS.items():
        gb = shape(fa)
        total = len(gb) + 1                      # + terminator
        ok = total <= slot
        allok &= ok
        hexs = "".join(f"{b:02x}" for b in gb) + f"{term:02x}"
        print(f"0x{addr:08X}  {en:<15} {fa:<12} {len(gb):>6} {total:>5}/{slot:<2} "
              f"{'OK' if ok else 'OVER':>4}  {hexs}")
    print("ALL FIT" if allok else "*** SOME OVERFLOW ***")


if __name__ == "__main__":
    main()
