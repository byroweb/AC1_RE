#!/usr/bin/env python3
"""
farsi_bake.py — build-time baker: Persian text -> ROM byte sequence.

Pipeline:  logical Persian (Unicode)
             -> farsi_shape.shape()   (contextual forms, ligatures, RTL order)
             -> glyph keys
             -> farsi_glyphs.index_map (key -> index)
             -> bytes (0x80 + index), space -> 0x20, ASCII literal -> its byte
             -> append '>' (0x3E) terminator

The emitted bytes drop straight into a ROM string slot; the modified draw_string
walks them right-to-left.  Strings are re-baked by re-running this tool.
"""

from farsi_shape import shape
from farsi_glyphs import index_map, encode_byte

# Variant letters the shaper can emit that the core 128-glyph sheet doesn't carry
# get aliased to a visually-acceptable core glyph (basic-font simplification).
ALIASES = {
    'alefhamza': 'alef', 'aleflhmz': 'alef', 'alefwasl': 'alef',
    'vavhmz': 'vav', 'yehmz': 'ye', 'kafar': 'kaf', 'year': 'ye',
    'temarb': 'he2', 'hamza': 'alef',   # standalone hamza -> bare alef (placeholder)
}

_IDX = index_map(include_extras=True)


def _resolve(key):
    """Map a shaped glyph key to a glyph index, applying aliases."""
    if key in _IDX:
        return _IDX[key]
    if '.' in key:
        name, form = key.split('.', 1)
        if name in ALIASES:
            alias = f'{ALIASES[name]}.{form}'
            if alias in _IDX:
                return _IDX[alias]
            # aliased letter may lack this form (R has no init/medi) -> fall back
            for f in ('fina', 'isol'):
                if f'{ALIASES[name]}.{f}' in _IDX:
                    return _IDX[f'{ALIASES[name]}.{f}']
    return None


# Per-glyph advances (for RTL width), loaded from the renderer metrics table.
import json as _json
_ADV = {int(k): v['adv'] for k, v in
        _json.load(open('/home/byron/Desktop/AC_1_USA_RE/farsi_metrics.json'))['0'].items()}


def bake(text, terminator=True, right_align=False, field_width=72):
    """Persian/mixed string -> bytes for a ROM string slot.
    right_align: prepend a `\\x01 <signed shift>` escape so the text's RIGHT edge
    lands at ctx->x + field_width (the menu field's right margin, RTL).
    shift = field_width - W; x ends at ctx->x + shift, so right edge = +field_width."""
    out = bytearray()
    unresolved = []
    for g in shape(text):
        if isinstance(g, tuple) and g[0] == 'lit':      # ASCII passthrough
            out.append(ord(g[1]) & 0xFF)
        elif g == 'space':
            out.append(0x20)
        else:
            idx = _resolve(g)
            if idx is None:
                unresolved.append(g)
                out.append(0x3F)                        # '?' placeholder
            else:
                out.append(encode_byte(idx))
    if right_align:
        W = 0
        for b in out:
            W += _ADV.get(b - 0x80, 8) if b >= 0x80 else 8   # glyph adv / 8 for space+latin
        shift = field_width - W                              # signed: +right / -left
        shift = max(-128, min(127, shift))
        out = bytearray([0x01, shift & 0xFF]) + out
    if terminator:
        out.append(0x3E)                                # '>'
    return bytes(out), unresolved


# --------------------------------------------------------------------------- #
if __name__ == '__main__':
    samples = [
        ('سلام',      'hello'),
        ('بازی',      'game'),
        ('بالا',      'up (lam-alef)'),
        ('مأموریت',   'mission (aliased hamza)'),
        ('کور ۱',     'core 1 (mixed digit)'),
    ]
    print(f"{'word':<10} {'meaning':<22} ROM bytes")
    print('-' * 70)
    for w, meaning in samples:
        data, unresolved = bake(w)
        hexs = ' '.join(f'{b:02X}' for b in data)
        print(f'{w:<10} {meaning:<22} {hexs}')
        if unresolved:
            print(f'{"":<33} unresolved: {unresolved}')
