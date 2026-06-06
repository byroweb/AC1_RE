/*
 * farsi_name_input.c — Farsi name-entry handler (AC1). ISOLATED letters, RTL.
 *
 * Called every frame from the cursor handler tail with (cursor*, parent*, buttons).
 *  - Edge-detect buttons (act only on press transition, PREVBTN scratch).
 *  - confirm(0x40): append selected glyph to LOGBUF (logical/typed order);
 *    write the display name REVERSED (RTL: first-typed ends up rightmost).
 *  - backspace(0x20): drop last; START(0x800): cursor -> END glyph (row3 col2).
 *  - END (seltab 0xFC): draw "Confirm name?" prompt (0x8005D8D4) + set confirm
 *    state (parent+116 = 0x8008240C), mirroring stock.
 *  SELTAB[row*17+col]: glyph byte / 0x20 space / 0xFC END / 0xFF blank.
 *  Buffers: SELTAB 0x8004C700, PREVBTN 0x8004C748, LOGBUF 0x8004C750,
 *           name AC 0x80031BD4 / PILOT 0x80031BE6.
 */
typedef unsigned char u8;
typedef unsigned int  u32;

#define SELTAB     ((const u8 *)0x8004c700)
#define PREVBTN    ((u32 *)0x8004c748)
#define LOGBUF     ((u8 *)0x8004c750)
#define NAME_AC    ((u8 *)0x80031bd4)
#define NAME_PILOT ((u8 *)0x80031be6)
#define TERM       0x3e
#define TOK_END    0xFC
#define TOK_BLANK  0xFF
#define NAME_MAX   8
#define CONFIRM_PROMPT 0x8004c878            /* "Confirm the registered name?" */
#define STATE_CONFIRM  0x8008240c

extern void draw_confirm(u32 str, u32 a1, u32 a2, u32 a3);   /* = 0x8005D8D4 */

static void rebuild(u8 *name, u8 len)        /* RTL: reverse LOGBUF into display */
{
    int i;
    for (i = 0; i < len; i++)
        name[i] = LOGBUF[len - 1 - i];
    name[len] = TERM;
}

void name_input_handler(u8 *cur, u8 *parent, u32 buttons)
{
    u32 edge = buttons & ~(*PREVBTN);
    u8  len;
    u8 *name;
    *PREVBTN = buttons;
    /* mark consumed action buttons in the game's latch so downstream states
       (e.g. the name-confirm prompt) don't see this same press as fresh */
    *(u32 *)0x801a2550 |= (edge & (0x40 | 0x20 | 0x800));

    if (edge & 0x800) {                       /* START -> jump to END glyph */
        cur[30] = 2;
        cur[31] = 3;
        return;
    }
    len  = cur[26];
    name = (cur[25] == 0x41) ? NAME_AC : NAME_PILOT;

    if (edge & 0x40) {                        /* confirm / select */
        u8 tok = SELTAB[cur[31] * 17 + cur[30]];
        if (tok == TOK_BLANK)
            return;
        if (tok == TOK_END) {
            draw_confirm(CONFIRM_PROMPT, (u32)(cur + 24), 12, 0);
            *(u32 *)(parent + 116) = STATE_CONFIRM;
            return;
        }
        if (len >= NAME_MAX)
            return;
        LOGBUF[len++] = tok;
        cur[26] = len;
        rebuild(name, len);
    } else if (edge & 0x20) {                 /* backspace */
        if (len == 0)
            return;
        cur[26] = --len;
        rebuild(name, len);
    }
}
