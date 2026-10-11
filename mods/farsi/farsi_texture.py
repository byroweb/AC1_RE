#!/usr/bin/env python3
"""
farsi_texture.py — render the 128-glyph Farsi sheet from a TTF (PIL + raqm).

First pass: produce a magnified GRAYSCALE preview so we can judge legibility and
style at the real 8x16 cell size BEFORE committing to the PS1 4bpp/CLUT format.
Also renders a few baked sample words (glyph cells blitted L->R by advance) to
show what connected text will look like in-game.
"""
from PIL import Image, ImageDraw, ImageFont
from farsi_glyphs import build_inventory, index_map
from farsi_bake import bake

ZWJ = '‍'
CELL_W, CELL_H = 8, 16
COLS = 16

# Map a glyph key -> the (ZWJ-wrapped) string that forces its contextual form.
LETTER_CP = {
    'alef':0x0627,'be':0x0628,'pe':0x067E,'te':0x062A,'se':0x062B,'jim':0x062C,
    'che':0x0686,'he':0x062D,'khe':0x062E,'dal':0x062F,'zal':0x0630,'re':0x0631,
    'ze':0x0632,'zhe':0x0698,'sin':0x0633,'shin':0x0634,'sad':0x0635,'zad':0x0636,
    'ta':0x0637,'za':0x0638,'eyn':0x0639,'gheyn':0x063A,'fe':0x0641,'ghaf':0x0642,
    'kaf':0x06A9,'gaf':0x06AF,'lam':0x0644,'mim':0x0645,'nun':0x0646,'vav':0x0648,
    'he2':0x0647,'ye':0x06CC,'alefmad':0x0622,
}
DIGIT_CP = {f'd{i}': 0x06F0+i for i in range(10)}

def form_string(key):
    if key.startswith('lamalef'):
        base = 'لا'  # lam+alef -> font ligates
        return ZWJ+base if key.endswith('fina') else base
    if key in DIGIT_CP:
        return chr(DIGIT_CP[key])
    name, form = key.split('.')
    c = chr(LETTER_CP[name])
    return {'isol':c, 'init':c+ZWJ, 'medi':ZWJ+c+ZWJ, 'fina':ZWJ+c}[form]

def render_cell(font, key, baseline, pen_x):
    """Render one glyph into an 8x16 L cell at a fixed baseline."""
    cell = Image.new('L', (CELL_W, CELL_H), 0)
    d = ImageDraw.Draw(cell)
    d.text((pen_x, baseline), form_string(key), fill=255, font=font, anchor='ls')
    return cell

def build_sheet(font_path, pt, baseline, pen_x):
    font = ImageFont.truetype(font_path, pt)
    inv = build_inventory(True)
    rows = (len(inv)+COLS-1)//COLS
    sheet = Image.new('L', (COLS*CELL_W, rows*CELL_H), 0)
    cells = {}
    for i, key in enumerate(inv):
        c = render_cell(font, key, baseline, pen_x)
        cells[key] = c
        cx, cy = (i%COLS)*CELL_W, (i//COLS)*CELL_H
        sheet.paste(c, (cx, cy))
    return sheet, cells, inv

def magnify(img, scale, grid=True):
    big = img.resize((img.width*scale, img.height*scale), Image.NEAREST)
    if grid:
        d = ImageDraw.Draw(big)
        for x in range(0, big.width+1, CELL_W*scale):
            d.line([(x,0),(x,big.height)], fill=64)
        for y in range(0, big.height+1, CELL_H*scale):
            d.line([(0,y),(big.width,y)], fill=64)
    return big

def render_word(cells, text, scale=8):
    """Blit baked glyph cells L->R (fixed cell advance) to preview connected text."""
    data, _ = bake(text, terminator=False)
    inv = build_inventory(True)
    strip = Image.new('L', (len(data)*CELL_W, CELL_H), 0)
    x = 0
    for b in data:
        if b == 0x20:
            x += CELL_W; continue
        key = inv[b-0x80] if b >= 0x80 else None
        if key and key in cells:
            strip.paste(cells[key], (x,0))
        x += CELL_W
    return strip.resize((strip.width*scale, strip.height*scale), Image.NEAREST)

if __name__ == '__main__':
    FONT = '/usr/share/fonts/truetype/noto/NotoSansArabic-Regular.ttf'
    PT, BASE, PENX = 15, 12, 1     # initial guess; tune from the preview
    sheet, cells, inv = build_sheet(FONT, PT, BASE, PENX)

    big = magnify(sheet, 8)
    big.save('/tmp/farsi_sheet.png')
    print(f'sheet: {sheet.size} -> /tmp/farsi_sheet.png ({big.size})')

    words = ['سلام','بازی','بالا','ایران','کور']
    strips = [render_word(cells, w) for w in words]
    W = max(s.width for s in strips); H = sum(s.height for s in strips)+ (len(strips)-1)*8
    board = Image.new('L',(W,H),0); y=0
    for s in strips:
        board.paste(s,(0,y)); y += s.height+8
    board.save('/tmp/farsi_words.png')
    print(f'words preview -> /tmp/farsi_words.png ({board.size})')
