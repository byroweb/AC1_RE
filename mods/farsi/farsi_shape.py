#!/usr/bin/env python3
"""
farsi_shape.py — Persian/Farsi contextual shaping pass for AC1 text rendering.

Given a logical-order Persian string (Unicode), produce the sequence of glyph
keys (letter + contextual form) in the order they must be DRAWN.  This is the
language-level core; it is independent of whether it runs at build time (baking
glyph indices into the ROM) or is later ported to C for runtime shaping.

Joining model (standard Arabic/Persian shaping):
  Each letter has a joining type:
    D = dual-joining   (4 forms: isolated/initial/medial/final)
    R = right-joining  (2 forms: isolated/final) — joins only to a preceding letter
    U = non-joining    (isolated only)
  A letter can:
    join-right (connect to the letter before it in reading order) iff D or R
    join-left  (connect to the letter after it in reading order)  iff D
  For current letter C with logical neighbours P (prev) and N (next):
    prev_conn = C.join_right and P is D            (P must be able to join-left)
    next_conn = C.join_left  and N is (D or R)     (N must be able to join-right)
  Form:
    prev_conn & next_conn -> medial
    prev_conn & !next      -> final
    !prev     & next_conn  -> initial
    !prev     & !next      -> isolated

Special case: LAM + ALEF is a mandatory ligature (lam-alef), drawn as ONE glyph
with two forms (isolated / final depending on whether the lam joins backward).
"""

# --- Joining-type table: Unicode codepoint -> 'D' | 'R' | 'U' -------------------
# Names kept short; suffix forms are appended by the shaper.
R = 'R'; D = 'D'; U = 'U'

LETTERS = {
    # right-joining (isolated + final only)
    0x0627: ('alef',      R), 0x0622: ('alefmad',  R),
    0x0623: ('alefhamza', R), 0x0625: ('aleflhmz', R), 0x0671: ('alefwasl', R),
    0x062F: ('dal',  R), 0x0630: ('zal',  R),
    0x0631: ('re',   R), 0x0632: ('ze',   R), 0x0698: ('zhe',  R),
    0x0648: ('vav',  R), 0x0624: ('vavhmz', R),
    0x0629: ('temarb', R),
    # non-joining
    0x0621: ('hamza', U),
    # dual-joining (4 forms)
    0x0628: ('be',  D), 0x067E: ('pe',  D), 0x062A: ('te',  D), 0x062B: ('se',  D),
    0x062C: ('jim', D), 0x0686: ('che', D), 0x062D: ('he',  D), 0x062E: ('khe', D),
    0x0633: ('sin', D), 0x0634: ('shin',D), 0x0635: ('sad', D), 0x0636: ('zad', D),
    0x0637: ('ta',  D), 0x0638: ('za',  D), 0x0639: ('eyn', D), 0x063A: ('gheyn',D),
    0x0641: ('fe',  D), 0x0642: ('ghaf',D), 0x06A9: ('kaf', D), 0x06AF: ('gaf', D),
    0x0644: ('lam', D), 0x0645: ('mim', D), 0x0646: ('nun', D), 0x0647: ('he2', D),
    0x06CC: ('ye',  D), 0x0626: ('yehmz', D),
    0x0643: ('kafar', D), 0x064A: ('year', D),  # arabic kaf/yeh fallbacks
}

# Persian + Arabic-Indic digits: standalone, no shaping
DIGITS = {
    0x06F0: 'd0', 0x06F1: 'd1', 0x06F2: 'd2', 0x06F3: 'd3', 0x06F4: 'd4',
    0x06F5: 'd5', 0x06F6: 'd6', 0x06F7: 'd7', 0x06F8: 'd8', 0x06F9: 'd9',
    0x0660: 'd0', 0x0661: 'd1', 0x0662: 'd2', 0x0663: 'd3', 0x0664: 'd4',
    0x0665: 'd5', 0x0666: 'd6', 0x0667: 'd7', 0x0668: 'd8', 0x0669: 'd9',
}

ALEFS = {0x0627, 0x0622, 0x0623, 0x0625, 0x0671}   # forms that ligate after lam
LAM   = 0x0644

FORM_SUFFIX = {'isol': '.isol', 'init': '.init', 'medi': '.medi', 'fina': '.fina'}


def _jtype(cp):
    """Joining type of a codepoint, or None if it's a break (space/punct/digit)."""
    if cp in LETTERS:
        return LETTERS[cp][1]
    return None  # everything else breaks the join run


def _name(cp):
    return LETTERS[cp][0]


def shape(text):
    """
    Shape a logical-order Persian string into a list of glyph keys in DRAW order
    (right-to-left: first logical char drawn rightmost -> appears last in list).

    Non-letters (space, digits, ASCII) pass through as ('lit', char) / ('digit', key).
    Returns a list of glyph-key strings (and literal tuples) in draw order.
    """
    cps = [ord(c) for c in text]
    n = len(cps)
    out = []           # in LOGICAL order first; reversed at the end for RTL draw
    i = 0
    while i < n:
        cp = cps[i]
        jt = _jtype(cp)

        # --- non-letters: digits, spaces, ASCII punctuation ---
        if jt is None:
            if cp in DIGITS:
                out.append(DIGITS[cp])
            elif cp == 0x20:
                out.append('space')
            else:
                out.append(('lit', chr(cp)))
            i += 1
            continue

        # neighbours for connection logic
        prev_cp = cps[i-1] if i > 0 else None
        next_cp = cps[i+1] if i+1 < n else None
        prev_jt = _jtype(prev_cp) if prev_cp is not None else None
        next_jt = _jtype(next_cp) if next_cp is not None else None

        prev_conn = (jt in (D, R)) and (prev_jt == D)
        next_conn = (jt == D)      and (next_jt in (D, R))

        # --- lam-alef mandatory ligature ---
        if cp == LAM and next_cp in ALEFS:
            # ligature form: final if lam joins backward, else isolated
            key = 'lamalef' + ('.fina' if prev_conn else '.isol')
            out.append(key)
            i += 2          # consume lam + alef
            continue

        # --- ordinary contextual form ---
        if   prev_conn and next_conn: form = 'medi'
        elif prev_conn:               form = 'fina'
        elif next_conn:               form = 'init'
        else:                         form = 'isol'
        out.append(_name(cp) + FORM_SUFFIX[form])
        i += 1

    out.reverse()       # logical -> RTL draw order
    return out


# --------------------------------------------------------------------------- #
if __name__ == '__main__':
    tests = [
        ('سلام',      'salaam / hello'),
        ('ایران',     'Iran'),
        ('بازی',      'baazi / game'),
        ('بالا',      'baalaa / up (lam-alef)'),
        ('خوش آمدید', 'welcome'),
        ('مأموریت',   'mission'),
        ('۱۲۳',       'digits 123'),
        ('کور',       'core'),
    ]
    print(f"{'word':<12} {'meaning':<24} draw-order glyphs (RTL)")
    print('-' * 90)
    for w, meaning in tests:
        glyphs = shape(w)
        pretty = ' '.join(g if isinstance(g, str) else f'<{g[1]}>' for g in glyphs)
        print(f'{w:<12} {meaning:<24} {pretty}')

    # distinct glyph inventory across the tests (texture-sizing signal)
    allg = set()
    for w, _ in tests:
        for g in shape(w):
            if isinstance(g, str):
                allg.add(g)
    print('\nDistinct glyph keys used in tests:', len(allg))
    print(sorted(allg))
