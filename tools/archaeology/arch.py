#!/usr/bin/env python3
"""arch.py - shared reader + per-function asm features for the code-archaeology study.

Reads tools/loop/functions.tsv and the unit images through tools/loop (units.py,
twins.func_words / twins.mask_function), so the game bytes stay in the
gitignored disc_map / tools/loop/cache files and nothing here embeds them.

    from arch import load_rows, features, SECTIONS

`features(unit, row)` decodes one function and returns a dict of counts that the
other scripts in this directory aggregate (see docs/CODE_ARCHAEOLOGY.md for what
each one is evidence of).  Run directly to dump the table:

    python tools/archaeology/arch.py [--units exe,ovl201] > features.tsv
"""
import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import twins  # noqa: E402

FUNCTIONS = os.path.join(REPO, "tools", "loop", "functions.tsv")
PROGRAM_UNITS = ("exe", "ovl201", "ovl202", "ovl203", "ovl204")

# (rodata_lo, text_lo, text_hi, image_hi) from overlays/CODE_UNITS.tsv
# (Wyrelade's v1.0 splat layout, code-identical to v1.1).  .data runs from
# text_hi to image_hi; anything above image_hi up to BSS_SPAN is taken as the
# unit's .bss.
SECTIONS = {
    "exe":    (0x80011000, 0x80011E64, 0x800315B4, 0x8003A000),
    "ovl201": (0x8004ADA0, 0x8004DCC0, 0x800B6DEC, 0x800CF5A0),
    "ovl202": (0x8004ADA0, 0x8004C1BC, 0x8008D91C, 0x80090DA0),
    "ovl203": (0x8004ADA0, 0x8004BE10, 0x800895C4, 0x8008C5A0),
    "ovl204": (0x8004ADA0, 0x8004BDB8, 0x800893C0, 0x8008C5A0),
}
BSS_SPAN = 0x40000
EXE_GAME = (0x80011F28, 0x80016EA4)      # exe game code: main .. start of PsyQ libs
SAVED = set(range(16, 24)) | {30, 31}


def section_of(unit, addr):
    """'rodata' | 'text' | 'data' | 'bss' | None (outside the unit)."""
    s = SECTIONS.get(unit)
    if not s:
        return None
    ro, tx, te, hi = s
    if ro <= addr < tx:
        return "rodata"
    if tx <= addr < te:
        return "text"
    if te <= addr < hi:
        return "data"
    if hi <= addr < hi + BSS_SPAN and unit != "exe":
        return "bss"
    if unit == "exe" and hi <= addr < 0x8004ADA0:
        return "bss"
    return None


def load_rows(units=None, path=FUNCTIONS):
    out = []
    with open(path, encoding="utf-8") as fh:
        header = fh.readline().rstrip("\r\n").split("\t")
        for line in fh:
            r = dict(zip(header, line.rstrip("\r\n").split("\t")))
            if units and r["unit"] not in units:
                continue
            r["va"] = int(r["vaddr"], 16)
            r["sz"] = int(r["size"])
            out.append(r)
    return out


def is_game(r):
    """exe rows outside the PsyQ block, every overlay row."""
    if r["unit"] == "exe":
        return r.get("is_psyq") == "0" and EXE_GAME[0] <= r["va"] < EXE_GAME[1]
    return True


FEATURES = (
    "n ld_b ld_h ld_w ld_bu ld_hu st_b st_h st_w lwlr sp_ld sp_st "
    "sext16 sext8 zext16 zext8 jt jalr vcall gte mul div brk mflo "
    "li_cmp slti bwd br jal j_tail nop dnop dslots saved fp gp "
    "frame leaf inc inc16 inc8").split()


def features(unit, r):
    words = twins.func_words(unit, r["va"], r["sz"])
    masked, slots = twins.mask_function(words, r["va"])
    f = dict.fromkeys(FEATURES, 0)
    f["n"] = len(words)
    f["frame"] = int(r.get("stack_size") or 0)
    f["jt"] = int(r.get("jump_tables") or 0)
    li = {}                 # reg -> index of `addiu r,$0,imm` / `ori r,$0,imm`
    lw_from_obj = {}        # reg -> index of `lw r,off(base)` with base not sp/gp
    shl = {}                # reg -> (index, shift) of the last sll
    va0 = r["va"]
    end = len(words)
    for i, w in enumerate(words):
        op, rs, rt, rd = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        sa, fn, imm = (w >> 6) & 31, w & 63, w & 0xFFFF
        if w == 0:
            f["nop"] += 1
        if op == 0:
            if fn == 0x00 and w:                                   # sll
                shl[rd] = (i, sa, rt)
            elif fn == 0x03:                                       # sra
                p = shl.get(rt)
                if p and p[1] == sa and i - p[0] <= 3:
                    if sa == 16:
                        f["sext16"] += 1
                    elif sa == 24:
                        f["sext8"] += 1
            elif fn == 0x09:                                       # jalr
                f["jalr"] += 1
                j = lw_from_obj.get(rs)
                if j is not None and i - j <= 6:
                    f["vcall"] += 1
            elif fn in (0x18, 0x19):
                f["mul"] += 1
            elif fn in (0x1A, 0x1B):
                f["div"] += 1
            elif fn == 0x0D:
                f["brk"] += 1
            elif fn in (0x10, 0x12):
                f["mflo"] += 1
            elif fn in (0x21, 0x25) and rd == 30 and rs == 29 and rt == 0:
                f["fp"] = 1
        elif op == 3:
            f["jal"] += 1
        elif op == 2 and i >= end - 2:
            f["j_tail"] += 1
        elif op in (0x04, 0x05, 0x06, 0x07, 0x01):                 # branches
            f["br"] += 1
            off = imm - 0x10000 if imm & 0x8000 else imm
            if off < 0:
                f["bwd"] += 1
            if op in (0x04, 0x05):
                for reg in (rs, rt):
                    k = li.get(reg)
                    if reg and k is not None and i - k <= 3:
                        f["li_cmp"] += 1
                        break
        elif op in (0x09, 0x0D) and rs == 0:                       # li
            li[rt] = i
        elif op == 0x09 and imm in (1, 0xFFFF) and rt not in (0, 29):  # counter step
            f["inc"] += 1
            for nw in words[i + 1:i + 4]:
                nop_, nrt, nfn = nw >> 26, (nw >> 16) & 31, nw & 63
                if nop_ == 0 and nfn == 0 and nrt == rt and ((nw >> 6) & 31) in (16, 24):
                    f["inc16" if ((nw >> 6) & 31) == 16 else "inc8"] += 1
                    break
                if nop_ == 0x0C and (nw >> 21) & 31 == rt and nw & 0xFFFF in (0xFF, 0xFFFF):
                    f["inc8" if nw & 0xFFFF == 0xFF else "inc16"] += 1
                    break
        elif op in (0x0A, 0x0B):
            f["slti"] += 1
        elif op == 0x0C and imm in (0xFFFF, 0xFF):
            f["zext16" if imm == 0xFFFF else "zext8"] += 1
        elif op == 0x12 or op in (0x32, 0x3A):
            f["gte"] += 1
        if op in (0x20, 0x21, 0x23, 0x24, 0x25, 0x22, 0x26, 0x28, 0x29, 0x2B, 0x2A, 0x2E):
            if rs == 29:
                if op == 0x23 and rt not in SAVED:
                    f["sp_ld"] += 1
                elif op == 0x2B and rt not in SAVED:
                    f["sp_st"] += 1
                elif op == 0x2B and rt in SAVED and i < 40:
                    f["saved"] += 1
                elif op not in (0x23, 0x2B):
                    f["sp_ld" if op < 0x28 else "sp_st"] += 1
            else:
                key = {0x20: "ld_b", 0x24: "ld_bu", 0x21: "ld_h", 0x25: "ld_hu", 0x23: "ld_w",
                       0x28: "st_b", 0x29: "st_h", 0x2B: "st_w"}.get(op, "lwlr")
                f[key] += 1
                if op == 0x23 and rs not in (28, 29):
                    lw_from_obj[rt] = i
        if rs == 28 and (0x08 <= op <= 0x0E or 0x20 <= op <= 0x2E):
            f["gp"] += 1
        # delay slots: the word after a branch / jump / jal
        if i > 0:
            pw = words[i - 1]
            pop = pw >> 26
            if pop in (1, 2, 3, 4, 5, 6, 7) or (pop == 0 and (pw & 63) in (8, 9)):
                f["dslots"] += 1
                if w == 0:
                    f["dnop"] += 1
        wr = twins.written_reg(w)
        if wr:
            if not (op in (0x09, 0x0D) and rs == 0):
                li.pop(wr, None)
            if not (op == 0x23 and rs not in (28, 29)):
                lw_from_obj.pop(wr, None)
            if not (op == 0 and fn == 0):
                shl.pop(wr, None)
    f["leaf"] = int(f["jal"] == 0 and f["jalr"] == 0)
    f["slots"] = slots
    return f


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--units", default=",".join(PROGRAM_UNITS))
    a = ap.parse_args(argv)
    units = a.units.split(",")
    print("\t".join(["unit", "vaddr", "size"] + FEATURES))
    for r in load_rows(units):
        if not is_game(r):
            continue
        f = features(r["unit"], r)
        print("\t".join([r["unit"], r["vaddr"], r["size"]] + [str(f[k]) for k in FEATURES]))


if __name__ == "__main__":
    main()
