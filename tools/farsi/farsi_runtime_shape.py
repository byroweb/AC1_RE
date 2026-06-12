#!/usr/bin/env python3
"""
farsi_runtime_shape.py — RUNTIME contextual shaper, reference implementation.

This is the on-console name shaper modelled exactly as the MIPS port will work,
so the C/asm version can be translated line-for-line and trusted.

Why a runtime shaper at all
---------------------------
The build-time shaper (farsi_shape.py) reshapes FIXED strings baked into the ROM.
Pilot/AC names are TYPED at runtime from isolated keyboard letters, so they must
be reshaped every frame they are displayed (input field, confirm screen,
rankings).  The keyboard stores a name as a sequence of 1-byte LETTER ORDINALS
(see KEYBOARD below); this module turns that buffer into the draw-order glyph
bytes that the patched draw_string consumes (byte >= 0x80 -> draw_farsi).

On-console data model (mirrors the MIPS tables)
-----------------------------------------------
  name buffer : u8[]   logical-order letter ordinals, 0..NLET-1, '>' (0x3E) term
  LTAB[ord]   : packed per-letter:
                  isol  (u8)  atlas glyph index of the isolated form (group base)
                  flags (u8)  bit0 = dual-joining (D); 1 => 4 forms, 0 => R/2 forms
                              bit1 = is LAM     (ligature trigger)
                              bit2 = is ALEF    (ligature target: alef / alef-madda)
  output      : glyph bytes (0x80+index) in RTL DRAW order + '>' terminator

Form selection (identical maths to farsi_shape.py)
--------------------------------------------------
  prev_conn = prev is a letter AND prev is dual-joining (D)
  next_conn = this is dual-joining (D) AND next is a letter
  D offset: isol=0 init=1 medi=2 fina=3, chosen by (prev_conn,next_conn)
  R offset: isol=0 fina=1, chosen by prev_conn (R never connects leftward)
  LAM + ALEF(+madda) -> lamalef.fina if prev_conn else lamalef.isol (consume both)

Run as a script to execute the validation harness against farsi_shape.py.
"""

import farsi_glyphs
import farsi_shape

IDX = farsi_glyphs.index_map(include_extras=True)   # glyph key -> atlas index


# --- KEYBOARD: the selectable letters, in keyboard/ordinal order ----------------
# (display name, unicode codepoint, joining type).  Ordinal = list position.
# This is the canonical Persian alphabet + alef-madda, matching farsi_glyphs.
D = 'D'; R = 'R'
KEYBOARD = [
    ('alef',   0x0627, R), ('be',   0x0628, D), ('pe',   0x067E, D),
    ('te',     0x062A, D), ('se',   0x062B, D), ('jim',  0x062C, D),
    ('che',    0x0686, D), ('he',   0x062D, D), ('khe',  0x062E, D),
    ('dal',    0x062F, R), ('zal',  0x0630, R), ('re',   0x0631, R),
    ('ze',     0x0632, R), ('zhe',  0x0698, R), ('sin',  0x0633, D),
    ('shin',   0x0634, D), ('sad',  0x0635, D), ('zad',  0x0636, D),
    ('ta',     0x0637, D), ('za',   0x0638, D), ('eyn',  0x0639, D),
    ('gheyn',  0x063A, D), ('fe',   0x0641, D), ('ghaf', 0x0642, D),
    ('kaf',    0x06A9, D), ('gaf',  0x06AF, D), ('lam',  0x0644, D),
    ('mim',    0x0645, D), ('nun',  0x0646, D), ('vav',  0x0648, R),
    ('he2',    0x0647, D), ('ye',   0x06CC, D), ('alefmad', 0x0622, R),
]

# Flags bits
F_DUAL = 0x01
F_LAM  = 0x02
F_ALEF = 0x04

# Build the runtime per-letter table LTAB[ord] = (isol_index, flags)
LTAB = []
for name, cp, jt in KEYBOARD:
    isol = IDX[f'{name}.isol']
    flags = 0
    if jt == D:
        flags |= F_DUAL
    if name == 'lam':
        flags |= F_LAM
    if name in ('alef', 'alefmad'):     # alef + alef-madda ligate after lam
        flags |= F_ALEF
    LTAB.append((isol, flags))

# unicode -> ordinal, for the validation harness only (not used on console)
CP2ORD = {cp: i for i, (_, cp, _) in enumerate(KEYBOARD)}

LAMALEF_ISOL = IDX['lamalef.isol']
LAMALEF_FINA = IDX['lamalef.fina']

NLET = len(KEYBOARD)            # logical tokens 0..NLET-1 are letters

# Non-letter keyboard keys = literal pass-throughs. A logical token >= NLET is an
# index into LITERALS; the shaper emits that output byte verbatim and BREAKS the
# join run (neighbouring letters don't connect across it).  Output bytes:
#   space 0x20; Persian digits ۰..۹ = atlas d0..d9 = 0xF6..0xFF; punctuation as
#   kept ASCII cells (period/parens) or Persian-punct glyphs drawn into spare
#   ASCII cells (comma ، / decimal ٫ / kashida ـ). Exact punct cell bytes are a
#   font-layout decision (placeholders below); only "passes through + breaks" is
#   what the shaper guarantees.
LIT_SPACE = NLET + 0
LIT_D0    = NLET + 1            # d0..d9 -> NLET+1 .. NLET+10
LIT_PERIOD= NLET + 11
LIT_LPAREN= NLET + 12          # '(' (screen-left); rendered flipped in font
LIT_RPAREN= NLET + 13          # ')'
LIT_COMMA = NLET + 14          # ، Persian comma
LIT_DECIM = NLET + 15          # ٫ Persian decimal
LIT_KASH  = NLET + 16          # ـ kashida
LITERALS = ([0x20] +
            [0xF6 + d for d in range(10)] +              # ۰..۹
            [0x2E, 0x28, 0x29, 0x2C, 0x60, 0x5F])       # . ( ) ، ٫ ـ (ASCII cells)


def _is_letter(tok):
    return tok < NLET


def shape_runtime(tokens):
    """
    tokens : list[int] logical-order tokens (letter ordinals 0..NLET-1, or
             literal indices >= NLET). This is the name's logical scratch buffer.
    returns: list[int] OUTPUT glyph bytes in RTL draw order.
    Mirrors the MIPS routine exactly: single left-to-right pass, then reverse.
    """
    out = []                      # logical order; reversed at end
    n = len(tokens)
    i = 0
    while i < n:
        o = tokens[i]

        # --- non-letter literal: emit verbatim, breaks the join run ---
        if not _is_letter(o):
            out.append(LITERALS[o - NLET])
            i += 1
            continue

        isol, flags = LTAB[o]
        dual = flags & F_DUAL

        prev_is_letter = (i > 0) and _is_letter(tokens[i-1])
        next_is_letter = (i + 1 < n) and _is_letter(tokens[i+1])
        prev_conn = 1 if (prev_is_letter and (LTAB[tokens[i-1]][1] & F_DUAL)) else 0
        next_conn = 1 if (dual and next_is_letter) else 0

        # lam + alef(+madda) mandatory ligature
        if (flags & F_LAM) and next_is_letter and (LTAB[tokens[i+1]][1] & F_ALEF):
            lig = LAMALEF_FINA if prev_conn else LAMALEF_ISOL
            out.append(0x80 + lig)
            i += 2
            continue

        if dual:
            # offset table indexed by (prev_conn<<1 | next_conn): isol/init/medi/fina
            off = (0, 1, 3, 2)[(prev_conn << 1) | next_conn]
        else:
            off = prev_conn        # R: isol(0) / fina(1)
        out.append(0x80 + isol + off)
        i += 1

    out.reverse()
    return out


# --------------------------------------------------------------------------- #
# Persian digit codepoints -> literal token (for the harness only)
_PDIGIT = {0x06F0 + d: LIT_D0 + d for d in range(10)}
_PUNCT  = {0x20: LIT_SPACE, ord('.'): LIT_PERIOD, ord('('): LIT_LPAREN,
           ord(')'): LIT_RPAREN, 0x060C: LIT_COMMA, 0x066B: LIT_DECIM,
           0x0640: LIT_KASH}
# build-time glyph key 'space'/'dN' -> output byte, to compare like-for-like
_BT_LIT = {'space': 0x20}


def _bake_via_buildtime(word):
    """Reference: build-time shaper -> output bytes (letters, digits, space)."""
    res = []
    for g in farsi_shape.shape(word):
        if isinstance(g, str) and g in IDX:        # letter or digit glyph
            res.append(0x80 + IDX[g])
        elif g in _BT_LIT:
            res.append(_BT_LIT[g])
        elif isinstance(g, tuple) and g[0] == 'lit':   # ASCII literal -> its byte
            res.append(ord(g[1]))
        else:
            raise AssertionError(f'unhandled build-time glyph {g!r}')
    return res


def _word_to_tokens(word):
    toks = []
    for c in word:
        cp = ord(c)
        if cp in CP2ORD:       toks.append(CP2ORD[cp])
        elif cp in _PDIGIT:    toks.append(_PDIGIT[cp])
        elif cp in _PUNCT:     toks.append(_PUNCT[cp])
        else:                  raise KeyError(hex(cp))
    return toks


if __name__ == '__main__':
    # letters-only (regression) + mixed letters/digits/space/punct
    tests = ['سلام', 'ایران', 'بازی', 'بالا', 'کور', 'مموریت', 'بابا', 'للا',
             'الار', 'نننن', 'بببب', 'ووو', 'لا', 'بالالا',
             'بار۲', 'ا۱ب', '۱۲۳', 'سلام دنیا', 'با لا', 'کور.']
    print(f"{'word':<12} {'tokens (logical)':<26} runtime bytes (RTL)        ok")
    print('-' * 82)
    allok = True
    for w in tests:
        try:
            toks = _word_to_tokens(w)
        except KeyError as e:
            print(f'{w:<12} SKIP (unmapped cp {e})'); continue
        rt  = shape_runtime(toks)
        ref = _bake_via_buildtime(w)
        ok  = rt == ref
        allok &= ok
        bytestr = ' '.join(f'{b:02X}' for b in rt)
        print(f'{w:<12} {str(toks):<26} {bytestr:<26} {"OK" if ok else "MISMATCH"}')
        if not ok:
            print(f'           expected: {" ".join(f"{b:02X}" for b in ref)}')
    print('-' * 82)
    print('ALL MATCH' if allok else 'FAILURES PRESENT')
