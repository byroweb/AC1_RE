#!/usr/bin/env python3
"""
title_wordart.py — render the six hub category titles as CRISP native-size Farsi
word-art for the AC1 [RTL] disc.

The hub title sprite draws each word 1:1 at **128x24** from a font-page texture
(see AC1_TITLE_TEXTURE.md: descriptor table @0x800B6E4C). The original English
word-art (MISSION/MAIL/GARAGE/RANKING/SHOP/SYSTEM) was pre-baked at that size.
We do the same for Farsi: full PIL+raqm Unicode shaping (hamza, ZWNJ, etc. all
fine — unlike the 128-glyph name atlas), quantized to 4bpp / CLUT 0x3817 (white
ink = index 1, transparent = index 0), one 128x24 cell per word.

Output: a 4bpp packed sheet (2 cols x 3 rows of 128x24 cells = 256x72) plus a
magnified PNG preview. Cell layout matches the planned free-VRAM placement
(tpage 11 @ VRAM x704,y128): u in {0,128}, v in {0,24,48} within the sub-sheet.
"""
from PIL import Image, ImageFont, ImageDraw

FONT = '/usr/share/fonts/truetype/noto/NotoSansArabic-Bold.ttf'
CELL_W, CELL_H = 128, 24
THRESHOLD = 128

# carousel category id -> Farsi word (ids per descriptor table @0x800B6E4C)
WORDS = {
    0: ('GARAGE',  'گاراژ'),
    1: ('SHOP',    'فروشگاه'),
    2: ('MISSION', 'مأموریت'),
    3: ('SYSTEM',  'سیستم'),
    4: ('MAIL',    'ایمیل'),
    5: ('RANKING', 'رده‌بندی'),
}

def render_cell(word, pt=20, dy=0):
    """Render one Farsi word centered in a 128x24 1-bit cell (white on black)."""
    # supersample x4 for clean shaping, then threshold down
    S = 4
    big = Image.new('L', (CELL_W*S, CELL_H*S), 0)
    d = ImageDraw.Draw(big)
    font = ImageFont.truetype(FONT, pt*S)
    # anchor='mm' centers; raqm handles RTL + contextual shaping automatically
    d.text((CELL_W*S//2, CELL_H*S//2 + dy*S), word, fill=255, font=font,
           anchor='mm')
    small = big.resize((CELL_W, CELL_H), Image.LANCZOS)
    return small.point(lambda v: 255 if v >= THRESHOLD else 0)

def fit_pt(word):
    """Pick the largest point size whose rendered ink fits within the cell."""
    for pt in range(22, 8, -1):
        c = render_cell(word, pt)
        bbox = c.getbbox()
        if bbox and (bbox[2]-bbox[0]) <= CELL_W-4 and (bbox[3]-bbox[1]) <= CELL_H-2:
            return pt
    return 10

if __name__ == '__main__':
    cells = {}
    for cid, (en, fa) in WORDS.items():
        pt = fit_pt(fa)
        cells[cid] = (en, fa, pt, render_cell(fa, pt))
        print(f'id{cid} {en:8s} {fa}  pt={pt}')

    # preview board (magnified, on the banner-ish dark bg) with English labels
    SC = 4
    board = Image.new('RGB', (CELL_W*SC+150, (CELL_H*SC+10)*6+10), (18, 10, 28))
    dr = ImageDraw.Draw(board)
    y = 8
    for cid in range(6):
        en, fa, pt, c = cells[cid]
        big = c.resize((CELL_W*SC, CELL_H*SC), Image.NEAREST).convert('RGB')
        board.paste(big, (140, y))
        dr.text((8, y+CELL_H*SC//2-6), f'id{cid} {en}', fill=(170, 170, 180))
        y += CELL_H*SC+10
    board.save('/tmp/title_crisp_preview.png')
    print('preview -> /tmp/title_crisp_preview.png')
