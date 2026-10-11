/*
 * farsi_name_input.c — Farsi name-entry handler (AC1), CONTEXTUAL SHAPING.
 *
 * Replaces the earlier "isolated glyphs, naive reverse" handler. Every keystroke
 * now reshapes the WHOLE logical name through shape_name() (contextual init/medi/
 * fina forms + lam-alef ligature, RTL draw order) into the existing name buffer.
 *
 * Live-verified wiring (DuckStation, slot 6 = RTL name screen):
 *   - Hook: trampoline @0x80082168 calls this with (a0=cur, a1=parent,
 *     a2=buttons=*0x801a2568), then `j 0x800823f4` (outer epilogue). ra=0x80082180.
 *   - cur = 0x801A28D4 on the name screen.  cur[0x1a] (=cur[26]) == 0x801A28EE is
 *     the COUNT the baked RTL cursor patch (0x80083a28) reads:
 *         cursor_X (0x801a7360) = 120 - n*16
 *         name_X   (0x801a7500) = 136 - n*16
 *     Confirmed live: n=1 -> 104 / 120.  So cur[26] MUST hold the *shaped* glyph
 *     count for right-justification to track lam-alef collapse correctly.
 *   - cur[0x19] (cur[25]) = mode (0x41 => AC name, else PILOT).
 *   - cur[0x1e]/cur[0x1f] (cur[30]/cur[31]) = cursor col/row.
 *
 * Because cur[26] now carries the SHAPED count (which differs from the number of
 * typed keys whenever lam-alef collapses 2 letters -> 1 glyph), the logical typed
 * length is kept in its own scratch byte TYPED_LEN. cur[26]==0 (engine sets it on
 * name-screen entry / after a full backspace) resynchronises TYPED_LEN to 0.
 *
 * SELTAB is now ORDINAL TOKENS (not glyph bytes): 0..32 letter ordinals
 * (farsi_runtime_shape.KEYBOARD order), 33 = space, 34..43 = Persian digits
 * ۰..۹, 0xFC = END, 0xFF = blank.  shape_name() turns letter/space/digit tokens
 * into draw-order glyph bytes; END/blank are intercepted here before shaping.
 *
 * Buffers: SELTAB 0x8004C700, PREVBTN 0x8004C748, TYPED_LEN 0x8004C74C,
 *          LOGBUF 0x8004C750, name AC 0x80031BD4 / PILOT 0x80031BE6.
 */
typedef unsigned char u8;
typedef unsigned int  u32;

#define SELTAB     ((const u8 *)0x8004c700)   /* ordinal tokens, 17 cols/row */
#define PREVBTN    ((u32 *)0x8004c748)
#define TYPED_LEN  ((u8  *)0x8004c74c)        /* logical typed-token count    */
#define LOGBUF     ((u8  *)0x8004c750)        /* logical-order typed tokens   */
#define NAME_AC    ((u8  *)0x80031bd4)
#define NAME_PILOT ((u8  *)0x80031be6)
#define TOK_END    0xFC
#define TOK_BLANK  0xFF
#define NAME_MAX   8
#define CONFIRM_PROMPT 0x8004c878            /* "Confirm the registered name?" */
#define STATE_CONFIRM  0x8008240c
#define BTN_LATCH      0x801a2550

extern void draw_confirm(u32 str, u32 a1, u32 a2, u32 a3);   /* = 0x8005D8D4 */
extern int  shape_name(const u8 *toks, int n, u8 *out);      /* = 0x80065DBC */

void name_input_handler(u8 *cur, u8 *parent, u32 buttons)
{
    u32 edge = buttons & ~(*PREVBTN);
    u8  tlen;
    u8 *name;
    *PREVBTN = buttons;
    /* mark consumed action buttons in the game's latch so downstream states
       (e.g. the name-confirm prompt) don't see this same press as fresh */
    *(u32 *)BTN_LATCH |= (edge & (0x40 | 0x20 | 0x800));
    /* NOTE: the engine parks the cursor off-grid at entry (col 16, row 7). That
       default is moved to ALEF (col 16, row 0) by a 1-byte init patch in
       build_rtl_patch.py, NOT here -- the handler must stay <= 432 B
       (0x80082190..0x80082340, where the row-0 keyboard glyph string begins). */

    if (edge & 0x800) {                       /* START -> jump to END glyph */
        cur[30] = 2;
        cur[31] = 3;
        return;
    }

    name = (cur[25] == 0x41) ? NAME_AC : NAME_PILOT;
    if (cur[26] == 0)                         /* fresh entry / emptied: resync */
        *TYPED_LEN = 0;
    tlen = *TYPED_LEN;

    if (edge & 0x40) {                        /* confirm / select */
        u8 tok = SELTAB[cur[31] * 17 + cur[30]];
        if (tok == TOK_BLANK)
            return;
        if (tok == TOK_END) {
            draw_confirm(CONFIRM_PROMPT, (u32)(cur + 24), 12, 0);
            *(u32 *)(parent + 116) = STATE_CONFIRM;
            return;
        }
        if (tlen >= NAME_MAX)
            return;
        LOGBUF[tlen++] = tok;
        *TYPED_LEN = tlen;
        cur[26] = (u8)shape_name(LOGBUF, tlen, name);   /* shaped glyph count */
    } else if (edge & 0x20) {                 /* backspace */
        if (tlen == 0)
            return;
        *TYPED_LEN = --tlen;
        cur[26] = (u8)shape_name(LOGBUF, tlen, name);
    }
}
