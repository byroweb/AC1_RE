#!/usr/bin/env python3
"""
farsi_atlas.py — render the large (Sans/PT12) Farsi glyph atlas + size-indexed
metrics table.  Validated model: each glyph is an ink sprite placed at
(pen + bearing_x, baseline - top); pen advances by `advance`.

Outputs:
  /tmp/farsi_atlas.png        magnified atlas preview
  farsi_metrics.json          size-indexed metrics (size 0 = large, filled;
                              sizes 4/6/8 reserved empty for the future small set)

Metrics per glyph: U,V (atlas position), W,H (ink size),
                   bx (bearing x from pen), top (px above baseline), adv (advance).
Atlas is laid out in 16px-tall rows, <=256px wide (one PS1 tpage region).
"""
from PIL import Image, ImageDraw, ImageFont
from farsi_glyphs import build_inventory
import farsi_texture as ft
import json

FONT = '/usr/share/fonts/truetype/noto/NotoSansArabic-Regular.ttf'
PT   = 12
CELL_H = 16            # = game LARGE height; ascent 11 + descent 5
ASC    = 11
ATLAS_W = 256
ROW_H   = CELL_H + 1   # 1px separator

font = ImageFont.truetype(FONT, PT)
BIGW, BIGH, PX, PY = 128, 64, 40, 40

def glyph(key):
    img = Image.new('L', (BIGW, BIGH), 0)
    ImageDraw.Draw(img).text((PX, PY), ft.form_string(key), fill=255, font=font, anchor='ls')
    bb = img.getbbox()
    adv = round(font.getlength(ft.form_string(key)))
    if not bb:
        return None, 0, 0, max(adv, 3)
    x0, y0, x1, y1 = bb
    return img.crop((x0, y0, x1, y1)), x0 - PX, PY - y0, adv

def build():
    inv = build_inventory(True)
    # render all ink cells + raw metrics
    raw = {}
    for k in inv:
        ink, bx, top, adv = glyph(k)
        raw[k] = dict(ink=ink, bx=bx, top=top, adv=adv,
                      w=(ink.width if ink else 0), h=(ink.height if ink else 0))
    # pack into atlas rows
    atlas = Image.new('L', (ATLAS_W, 1), 0)
    x = y = 0; rows_h = ROW_H
    placed = {}
    for i, k in enumerate(inv):
        m = raw[k]
        w = max(m['w'], 1)
        if x + w > ATLAS_W:
            x = 0; y += ROW_H
        if y + ROW_H > atlas.height:
            atlas = atlas.crop((0, 0, ATLAS_W, y + ROW_H))
        if m['ink']:
            # place ink so its top sits at row baseline-ASC + (ASC-top)
            atlas.paste(m['ink'], (x, y + (ASC - m['top'])))
        placed[k] = dict(U=x, V=y, W=m['w'], H=m['h'],
                         bx=m['bx'], top=m['top'], adv=m['adv'])
        x += w + 1
    return inv, placed, atlas

def main():
    inv, placed, atlas = build()
    print(f'atlas: {atlas.size}  ({len(inv)} glyphs, {PT}pt, cell {CELL_H}px)')
    big = atlas.resize((atlas.width*4, atlas.height*4), Image.NEAREST)
    big.save('/tmp/farsi_atlas.png')
    print('preview -> /tmp/farsi_atlas.png', big.size)

    # size-indexed table: size 0 = large (filled); 4/6/8 reserved (empty)
    table = {'0': {str(i): placed[k] for i, k in enumerate(inv)},
             '4': {}, '6': {}, '8': {}}
    json.dump(table, open('/home/byron/Desktop/AC_1_USA_RE/farsi_metrics.json', 'w'),
              ensure_ascii=False)
    print('metrics -> farsi_metrics.json (size 0 filled; 4/6/8 reserved)')
    # quick stats
    advs = [placed[k]['adv'] for k in inv]
    print(f'advance min/max/avg: {min(advs)}/{max(advs)}/{sum(advs)//len(advs)}px; '
          f'atlas fits {atlas.height}px tall ({atlas.height/256:.2f} of a tpage)')

if __name__ == '__main__':
    main()
