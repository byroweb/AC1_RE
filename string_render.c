/*
 * string_render.c — AC1 USA (v1.1) string renderer, RE'd from FDAT entry 201
 *
 * draw_string() walks a Shift-JIS string stored in ctx->str and calls
 * draw_char() for each printable single-byte glyph.
 *
 * String encoding
 * ---------------
 *   Terminator  : '>' (0x3E)
 *   Encoding    : Shift-JIS — ASCII 0x20–0x7F, katakana 0xA0–0xDF as single
 *                 bytes; lead bytes 0x81–0x9F and 0xE0–0xFF begin 2-byte kanji.
 *
 * Control escapes (inline in the string)
 * ---------------------------------------
 *   0x0D + <skip>  carriage-return: x = x_start, y += 16, advance 2 bytes
 *   ';'            newline: x = x_start, y += 16 (mode 0) or 8 (modes 4/6/8)
 *   ' '            space: x += 8, no glyph
 *   '{' + digit    x -= (digit − '0')
 *   '}' + digit    x += (digit − '0')
 *   '^' + digit    set size_mode = (digit − '0')
 *   '~'            y -= 4
 *   '@' + digit    set palette = (digit − '0')
 *
 * RAM address  : 0x8006599C
 * Entry 201 offset: +0x1ABFC
 * Only caller  : 0x8009C1FC (within entry 201)
 * Calls        : draw_char @ 0x800660C4
 *                draw_kanji @ 0x80065DBC (2-byte path, dead code in USA)
 */

#include "psx_shim.h"

/* -------------------------------------------------------------------------
 * FontCtx — the context struct shared with draw_char
 *
 * Fields used by draw_char are documented in font_render.c.
 * Fields additionally used by draw_string:
 *   +0x0C  u_char *str      pointer to the current string
 *   +0x48  short x          starting x position
 *   +0x4C  short y          starting y position
 * ---------------------------------------------------------------------- */
typedef struct {
    u_char  _pad0[8];
    u_short zone;           /* +0x08: zone (0–20) → font style via table   */
    u_char  _pad1[2];       /* +0x0A */
    u_char *str;            /* +0x0C: string pointer (Shift-JIS + escapes) */
    u_char  _pad2[4];       /* +0x10 */
    u_short z_depth;        /* +0x14: OT Z-depth bucket                    */
    u_short render_mode;    /* +0x16: 0x0015 → fixed colour; else GTE      */
    u_long *ot[12];         /* +0x18: OT base ptrs, indexed by font_variant */
                            /*        12 × 4 = 48 bytes → next field @ +0x48 */
    short   x;              /* +0x48: start x for this string              */
    short   _pad3;          /* +0x4A */
    short   y;              /* +0x4C: start y for this string              */
} FontCtx;

/* -------------------------------------------------------------------------
 * Farsi glyph support
 *   Bytes >= 0x80 are Farsi glyphs (index = byte - 0x80).  Glyphs live in the
 *   lower region of the font texture (tpage 0x0007, CLUT 0x3817), packed
 *   variable-width.  fmet[] holds renderer-ready metrics; baked strings emit
 *   glyphs in visual L->R order so the normal left-to-right pen works (RTL is
 *   baked in at build time).
 * ---------------------------------------------------------------------- */
#include "farsi_table.h"        /* const FMet fmet[128] = { u,v,w,h,dx,dy,adv } */

/* Textured variable-size rectangle (GP0 0x64), 20 bytes / 5 u_longs, ilen 4 */
typedef struct {
    u_long  tag;               /* +0x00 OT next | ilen<<24                    */
    u_char  r, g, b, code;     /* +0x04 colour + code (0x64)                  */
    short   x, y;              /* +0x08 screen pos                            */
    u_char  u, v;              /* +0x0C UV in font texture                    */
    u_short clut;              /* +0x0E CLUT base                             */
    short   w, h;              /* +0x10 glyph w/h                             */
} SPRT_VAR;

#define FONT_CBA_MENU  0x3817

extern u_long *prim_ptr;       /* 0x801EF6CC: primitive write pointer         */
extern u_char  font_variant;   /* 0x801EF6C8: active font bank (0 = main)     */
extern void    set_font_drawmode(FontCtx *ctx);   /* 0x80066898              */

/* Font position-metrics table (resident @ 0x801BCE88, from FDAT entry 7).
 * draw_char maps ctx coords to SCREEN via adj/ref; the Farsi path MUST apply
 * the same transform: screen = adj + (coord - ref), else glyphs land off-screen. */
typedef struct {
    short  adj_x, adj_y;       /* +0x00, +0x02 */
    u_char _pad[4];            /* +0x04 */
    short  ref_x, ref_y;       /* +0x08, +0x0A */
} FontMetrics;
#define FONT_METRICS_BASE       0x801BCE88
#define METRICS_STRIDE_STYLE    100
#define METRICS_STRIDE_VARIANT  103512
static const u_char zone_to_style[21] = {
    0,0,0, 1,1, 2,2, 3,3, 4,4, 5,5, 6,6, 7,7, 8,8, 9,9,
};

/* draw_farsi is DEFINED at the end of this file so that draw_string remains
 * the first function in .text (the caller jumps to its fixed entry address).
 * noinline: x/y must be passed as real arguments (see definition). */
static void __attribute__((noinline))
draw_farsi(FontCtx *ctx, u_char gidx, short x, short y);

/* -------------------------------------------------------------------------
 * Extern functions
 * ---------------------------------------------------------------------- */
extern void draw_char(FontCtx *ctx, u_char ch, short x, short y,
                      u_char size_mode, u_char palette);

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

/*
 * glyph_width — x-advance after drawing one character with draw_char.
 * Matches the post-draw advance in the original string renderer loop.
 */
static inline short glyph_width(u_char mode)
{
    switch (mode) {
    case 4: return 4;
    case 6: return 6;
    default: return 8;   /* mode 0 and mode 8 both use 8-pixel advance */
    }
}

/*
 * is_kanji_lead — true for Shift-JIS 2-byte lead bytes:
 *   0x81–0x9F  and  0xE0–0xFF
 * The complement (0xA0–0xDF = katakana) is rendered via the single-byte path.
 *
 * Equivalent to: ch >= 0x80 && (u_char)(ch + 0x60) >= 0x40
 */
static inline int is_kanji_lead(u_char ch)
{
    return ch >= 0x80 && (u_char)(ch + 0x60u) >= 0x40u;
}

/* -------------------------------------------------------------------------
 * draw_string
 *
 * Renders the string in ctx->str starting at (ctx->x, ctx->y).
 * Processes escape sequences, calls draw_char for each printable glyph,
 * and advances the cursor according to glyph width and line breaks.
 * ---------------------------------------------------------------------- */
void draw_string(FontCtx *ctx)
{
    u_char *str      = ctx->str;
    short   x        = ctx->x;
    short   y        = ctx->y;
    short   x_start  = x;    /* saved once; restored by CR / semicolon     */
    u_char  size_mode = 0;
    u_char  palette   = 0;
    u_char  ch;

    while ((ch = *str) != '>') {

        if (ch == 0x01) {                /* RTL align: x += signed next byte */
            str++;
            x      += (signed char)*str++;
            x_start = x;

        } else if (ch == '\r') {         /* 0x0D — carriage return          */
            str    += 2;                 /* skip CR + the following byte    */
            x       = x_start;
            y      += 16;

        } else if (ch == ';') {          /* 0x3B — newline                  */
            str++;
            x = x_start;
            switch (size_mode) {
            case 0:           y += 16; break;
            case 4: case 6: case 8: y += 8;  break;
            /* modes 1,2,3: no y advance (not valid modes in practice) */
            }

        } else if (ch == ' ') {          /* 0x20 — space (no glyph)        */
            str++;
            x += 8;

        } else if (ch == '{') {          /* x -= digit                      */
            str++;
            x -= (short)(*str++ - '0');

        } else if (ch == '}') {          /* x += digit                      */
            str++;
            x += (short)(*str++ - '0');

        } else if (ch == '^') {          /* set size_mode                   */
            str++;
            size_mode = *str++ - '0';

        } else if (ch == '~') {          /* y -= 4                          */
            str++;
            y -= 4;

        } else if (ch == '@') {          /* set palette                     */
            str++;
            palette = *str++ - '0';

        } else if (ch < 0x80) {
            /* ASCII printable (0x20–0x7F) — the game's Latin font.
             * draw_char handles the UV lookup and primitive allocation. */
            str++;
            draw_char(ctx, ch, x, y, size_mode, palette);
            x += glyph_width(size_mode);

        } else {
            /* Farsi glyph: byte >= 0x80, index = byte - 0x80.
             * Glyph is pre-shaped + visual-L->R ordered at build time. */
            u_char gidx = (u_char)(ch - 0x80);
            str++;
            draw_farsi(ctx, gidx, x, y);
            x += fmet[gidx].adv;
        }
    }
}

/* -------------------------------------------------------------------------
 * draw_farsi — emit one Farsi glyph sprite at pen (x,y) using its metrics.
 * Placed last so draw_string stays first in .text.
 * ---------------------------------------------------------------------- */
static void __attribute__((noinline))
draw_farsi(FontCtx *ctx, u_char gidx, short x, short y)
{
    const FMet *m   = &fmet[gidx];
    int dx = (m->dydx & 0x10) ? -1 : 0;          /* unpack bearing */
    int dy = m->dydx & 0x0F;
    /* same per-zone style metrics draw_char uses (variant 0 / bank 0) */
    int moff = zone_to_style[ctx->zone] * METRICS_STRIDE_STYLE;
    const FontMetrics *fm = (const FontMetrics *)(FONT_METRICS_BASE + moff);
    SPRT_VAR   *prim = (SPRT_VAR *)prim_ptr;
    prim_ptr += 5;

    prim->code = 0x64;
    setlen(prim, 4);
    prim->r = prim->g = prim->b = 0x80;          /* white (render_mode 0x15) */
    /* same screen transform draw_char applies, plus this glyph's bearing */
    prim->x = fm->adj_x + (x - fm->ref_x) + dx;
    prim->y = fm->adj_y + (y - fm->ref_y) + dy;
    prim->u = m->u;
    prim->v = m->v;
    prim->w = m->w;
    prim->h = m->h;
    prim->clut = FONT_CBA_MENU;

    AddPrim(ctx->ot[font_variant] + ctx->z_depth, prim);
    set_font_drawmode(ctx);
}
