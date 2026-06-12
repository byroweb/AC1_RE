#!/usr/bin/env python3
"""
build_pdigits.py — replace the Western digit cells in the MENU_TIM font with
Persian digits ۰–۹, so the stock number-rendering (credits/AP/stats/rankings,
draw_char from ASCII '0'-'9' cells) shows Persian numerals with NO code change.

Input  : /tmp/MENU_TIM.T   (current modified font: Latin V0-95 + Farsi atlas V96-191)
Output : /tmp/MENU_TIM_pdigits.T  (same size, digits cells overwritten)
Inject : psximager .../psxinject "<BIN>" GG/MS/MENU_TIM.T /tmp/MENU_TIM_pdigits.T

CRITICAL: MENU_TIM.T entry 0 (the font) HAS a trailing checksum (same scheme as
FDAT overlays — see project_ac1_overlay_checksum). Entry 0 = sectors 1..34, base
file off 0x800, trailing word @0x117FC = 0x12345678 + sum(words[0x800..0x117F8]).
Editing the font WITHOUT recomputing this -> game hangs at "NOW LOADING".
fix_entry0_checksum() below handles it; call it before injecting.

Font texture facts (verified):
  embedded TIM @ file off 0x2E20 in MENU_TIM.T; 256x192 4bpp; CLUT(368,224)=16 cols;
  1-bit usage in Latin region (idx0 = transparent bg, idx1 = white ink).
  Image data @ 0x2E20 + 8(magic+flag) + 44(CLUT blk) + 12(img hdr) = 0x2E60.
  Row stride = 256/2 = 128 bytes; nibble order = low nibble is even-x pixel.

Digit cells: ASCII '0'-'9' = 0x30-0x39 -> col = (code-0x20)&0x1F = 16..25 -> U=col*8.
  Bands: LARGE V0 (8x16), SMALL8 V48 (8x8), SMALL6 V72 (6x8). (SMALL4 V96 is now
  Farsi atlas, so no tiny digits — acceptable.)
"""
import shutil
import struct
from PIL import Image, ImageFont, ImageDraw

E0_BASE, E0_TRAIL = 0x800, 0x117FC      # MENU_TIM.T entry-0 checksum span


def fix_entry0_checksum(d):
    """Recompute MENU_TIM.T entry-0 trailing checksum in-place (bytearray d)."""
    s = 0x12345678
    for off in range(E0_BASE, E0_TRAIL, 4):
        s = (s + struct.unpack_from('<I', d, off)[0]) & 0xFFFFFFFF
    struct.pack_into('<I', d, E0_TRAIL, s)
    return s

SRC = '/tmp/MENU_TIM.T'
DST = '/tmp/MENU_TIM_pdigits.T'
FONT = '/usr/share/fonts/truetype/noto/NotoSansArabic-Bold.ttf'   # matches Farsi set
IMG_OFF, W, H = 0x2E60, 256, 192
ROW = W // 2
PDIG = [chr(0x06F0 + i) for i in range(10)]          # ۰..۹

# (V band, cell W, cell H, font pt, threshold, baseline nudge dy)
# dy raised (was +2) per feedback that in-game numbers sat too low.
BANDS = [(0, 8, 16, 13, 90, -1), (48, 8, 8, 8, 90, -1), (72, 6, 8, 7, 90, -1)]


def main():
    shutil.copyfile(SRC, DST)
    d = bytearray(open(DST, 'rb').read())

    def setpx(x, y, idx):
        i = IMG_OFF + y * ROW + (x // 2)
        b = d[i]
        d[i] = (b & 0xF0) | (idx & 0xF) if x % 2 == 0 else (b & 0x0F) | ((idx & 0xF) << 4)

    def cell(ch, w, h, pt, thr, dy):
        big = Image.new('L', (w * 4, h * 4), 0)
        ImageDraw.Draw(big).text((w * 2, h * 2 + dy * 4), ch, fill=255,
                                 font=ImageFont.truetype(FONT, pt * 4), anchor='mm')
        return big.resize((w, h), Image.LANCZOS).point(lambda v: 1 if v >= thr else 0)

    for (V, cw, ch, pt, thr, dy) in BANDS:
        for di, c in enumerate(PDIG):
            U = (16 + di) * 8
            m = cell(c, cw, ch, pt, thr, dy)
            for yy in range(ch):
                for xx in range(8):           # clear full 8-wide cell
                    setpx(U + xx, V + yy, 0)
                for xx in range(cw):
                    if m.getpixel((xx, yy)):
                        setpx(U + xx, V + yy, 1)
    ck = fix_entry0_checksum(d)
    open(DST, 'wb').write(d)
    print(f'wrote {DST} ({len(d)} bytes), entry-0 checksum {ck:#010x}')


if __name__ == '__main__':
    main()
