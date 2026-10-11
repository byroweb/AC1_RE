#!/usr/bin/env python3
"""
farsi_glyphs.py — canonical Farsi glyph inventory + index/encoding assignment.

This is the contract shared by:
  - the shaper (farsi_shape.py) which emits glyph KEYS,
  - the texture sheet (each glyph index -> a cell / UV),
  - the modified draw_string (byte -> glyph index -> UV + width -> draw).

Encoding: each Farsi glyph index G (0..N-1) is stored in a ROM string as the
single byte (0x80 + G).  draw_string treats any byte >= 0x80 as a Farsi glyph
(replacing the dead kanji-lead path).  Space is NOT a glyph — it stays the
existing 0x20 advance.  So the whole inventory must fit in 0x80..0xFF (<=128).

Form rules:  D -> isol/init/medi/fina,  R -> isol/fina,  U -> isol.
"""

D = 'D'; R = 'R'; U = 'U'

# Core 32-letter Persian alphabet, in alphabetical order.
# (name, joining-type)  — names match farsi_shape.LETTERS
PERSIAN = [
    ('alef', R), ('be',  D), ('pe',  D), ('te',  D), ('se',  D), ('jim', D),
    ('che',  D), ('he',  D), ('khe', D), ('dal', R), ('zal', R), ('re',  R),
    ('ze',   R), ('zhe', R), ('sin', D), ('shin',D), ('sad', D), ('zad', D),
    ('ta',   D), ('za',  D), ('eyn', D), ('gheyn',D),('fe',  D), ('ghaf',D),
    ('kaf',  D), ('gaf', D), ('lam', D), ('mim', D), ('nun', D), ('vav', R),
    ('he2',  D), ('ye',  D),
]

# Optional extras (toggle as the font grows). alef-madda (آ) is common.
EXTRAS = [
    ('alefmad', R),   # آ  (2 forms) -> brings core to 128, the single-byte cap
]

FORMS = {D: ['isol', 'init', 'medi', 'fina'],
         R: ['isol', 'fina'],
         U: ['isol']}

LIGATURES = ['lamalef.isol', 'lamalef.fina']
DIGITS    = [f'd{i}' for i in range(10)]


def build_inventory(include_extras=True):
    """Return an ordered list of glyph keys; index = position in the list."""
    glyphs = []
    letters = PERSIAN + (EXTRAS if include_extras else [])
    for name, jt in letters:
        for form in FORMS[jt]:
            glyphs.append(f'{name}.{form}')
    glyphs += LIGATURES
    glyphs += DIGITS
    return glyphs


def index_map(include_extras=True):
    """glyph key -> index (0-based)."""
    return {g: i for i, g in enumerate(build_inventory(include_extras))}


def encode_byte(index):
    """Glyph index -> ROM byte."""
    return 0x80 + index


# --------------------------------------------------------------------------- #
if __name__ == '__main__':
    for extras in (False, True):
        inv = build_inventory(extras)
        n = len(inv)
        last = encode_byte(n - 1)
        tag = 'core+extras' if extras else 'core 32'
        fits = 'FITS' if last <= 0xFF else 'OVERFLOWS 0xFF'
        print(f'[{tag}] {n} glyphs -> bytes 0x80..0x{last:02X}  ({fits})')

    print()
    inv = build_inventory(True)
    idx = index_map(True)
    print(f'Full inventory ({len(inv)} glyphs), index : byte : key')
    cols = 4
    for i, g in enumerate(inv):
        print(f'  {i:3d}:0x{encode_byte(i):02X}:{g:<16}', end='')
        if i % cols == cols - 1:
            print()
    print()

    # Texture sheet sizing at 8x16 px/cell, 16 cells per row, 4bpp
    n = len(inv)
    cells_per_row = 16
    rows = (n + cells_per_row - 1) // cells_per_row
    w_px = cells_per_row * 8
    h_px = rows * 16
    bytes_4bpp = w_px * h_px // 2
    print(f'\nTexture (8x16 cells, {cells_per_row}/row): {rows} rows -> '
          f'{w_px}x{h_px}px, 4bpp = {bytes_4bpp:,} bytes ({bytes_4bpp/2048:.1f} sectors)')
