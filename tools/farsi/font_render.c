/*
 * font_render.c — AC1 USA (v1.1) text rendering, RE'd from FDAT entry 201
 *
 * Font texture: MENU_TIM.T[1], 4bpp, 256×192 px
 *   VRAM position : dx=448, dy=0   → tpage = 0x0007
 *   CLUT (menu)   : cx=368, cy=224 → cba   = 0x3817
 *   CLUT (HUD)    : cx=352, cy=240 → cba   = 0x3C16
 *
 * Character encoding:
 *   col = (char_code - 0x20) & 0x1F       U = col * 8
 *   row = (char_code - 1) >> 5
 *   V base by size mode (see FONT_MODE_* constants)
 */

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

/* tpage for 4bpp font at VRAM (448, 0), no semi-trans */
#define FONT_TPAGE       0x0007

/* CLUT base addresses */
#define FONT_CBA_MENU    0x3817   /* VRAM (368, 224) — white, used in menus  */
#define FONT_CBA_HUD     0x3C16   /* VRAM (352, 240) — coloured, used in HUD */

/* GPU primitive code for textured variable-size rectangle */
#define GP0_TEXRECT      0x64

/* Size modes — passed as the size_mode argument */
#define FONT_MODE_LARGE   0   /* 8×16 px — main menu / part names          */
#define FONT_MODE_SMALL8  8   /* 8×8  px                                    */
#define FONT_MODE_SMALL6  6   /* 6×8  px                                    */
#define FONT_MODE_SMALL4  4   /* 4×8  px — y is shifted +4 on screen        */

/* Zone-to-style map: ctx->zone (0–20) → font_style index (0–9).
   Each style selects a row in the metrics table. */
static const u_char zone_to_style[21] = {
    0, 0, 0,   /* zones  0– 2 */
    1, 1,      /* zones  3– 4 */
    2, 2,      /* zones  5– 6 */
    3, 3,      /* zones  7– 8 */
    4, 4,      /* zones  9–10 */
    5, 5,      /* zones 11–12 */
    6, 6,      /* zones 13–14 */
    7, 7,      /* zones 15–16 */
    8, 8,      /* zones 17–18 */
    9, 9,      /* zones 19–20 */
};

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

/*
 * Textured variable-size rectangle (GP0 command 0x64).
 * This is what the PSX SDK calls SPRT; the 'w' and 'h' fields make it
 * variable-size (unlike SPRT_8 / SPRT_16 which are fixed).
 * Total: 5 u_longs = 20 bytes, ilen = 4.
 */
typedef struct {
    u_long  tag;              /* +0x00: OT next-ptr (24-bit) | ilen (8-bit)  */
    u_char  r, g, b;          /* +0x04: colour modulation (0x80 = neutral)   */
    u_char  code;             /* +0x07: 0x64 = textured variable rect        */
    short   x, y;             /* +0x08: screen position (top-left)           */
    u_char  u, v;             /* +0x0C: UV in font texture                   */
    u_short clut;             /* +0x0E: CLUT base address (CBA)              */
    short   w, h;             /* +0x10: glyph width / height in pixels       */
} SPRT_VAR;                   /* 20 bytes                                    */

/*
 * Font metrics entry — 12 bytes per (style, variant) slot.
 * Located in the data overlay loaded alongside the code at ~0x801BCE88.
 * Stride: 100 bytes per style, 103512 bytes per variant bank.
 */
typedef struct {
    short  adj_x;    /* +0x00: x adjustment applied to the draw position    */
    short  adj_y;    /* +0x02: y adjustment applied to the draw position    */
    u_char pad[4];   /* +0x04: (unused in this path)                        */
    short  ref_x;    /* +0x08: reference x used to compute the adjustment   */
    short  ref_y;    /* +0x0A: reference y used to compute the adjustment   */
} FontMetrics;

#define FONT_METRICS_BASE          ((FontMetrics *)0x801BCE88)
#define FONT_METRICS_STRIDE_STYLE  100
#define FONT_METRICS_STRIDE_VARIANT 103512  /* bytes between variant banks  */

/*
 * Font context block — passed in as the first argument.
 * Only the fields observed in draw_char are described here.
 */
typedef struct {
    u_char  _pad0[8];
    u_short zone;       /* +0x08: zone index (0–20) → maps to style 0–9    */
    u_char  _pad1[10];
    u_short z_depth;    /* +0x14: OT Z-depth bucket index                  */
    u_short render_mode;/* +0x16: 0x0015 → use fixed colour; else use GTE  */
    u_long  *ot[1];     /* +0x18: per-variant pointers to OT arrays         */
} FontCtx;

/* -------------------------------------------------------------------------
 * Globals
 * ---------------------------------------------------------------------- */

extern u_long   *prim_ptr;      /* 0x801EF6CC: current position in prim buf */
extern u_char    font_variant;  /* 0x801EF6C8: active font bank (0 = main)  */

/* GTE lighting data used when render_mode != 0x0015 */
extern SVECTOR   gfx_normal;    /* 0x800B8D78 */
extern CVECTOR   gfx_color_in;  /* 0x800B8D80 */

/* -------------------------------------------------------------------------
 * Forward declarations
 * ---------------------------------------------------------------------- */
static void set_font_drawmode(FontCtx *ctx);

/* -------------------------------------------------------------------------
 * draw_char
 *
 * Renders one character into the primitive buffer and links it into the
 * ordering table.
 *
 *   ctx       — font context (zone, z_depth, OT pointers, …)
 *   char_code — ASCII character to draw (printable range 0x20–0x7E)
 *   x, y      — top-left screen position before metrics adjustment
 *   size_mode — FONT_MODE_LARGE / SMALL8 / SMALL6 / SMALL4
 *   palette   — 0 = menu CLUT (white), non-zero = HUD CLUT (colour)
 * ---------------------------------------------------------------------- */
void draw_char(FontCtx *ctx, u_char char_code, short x, short y,
               u_char size_mode, u_char palette)
{
    SPRT_VAR   *prim;
    FontMetrics *metrics;
    int          metrics_offset;
    u_char       font_style;
    u_char       col;           /* column in font grid (0–31)               */
    u_char       u0, v0;        /* UV coords within font texture             */
    short        glyph_w, glyph_h;
    u_long      *ot_entry;

    /* ------------------------------------------------------------------
     * 1. Compute texture U from character column.
     *    Characters are laid out in rows of 32, 8 pixels wide each.
     * ------------------------------------------------------------------ */
    col = (char_code - 0x20) & 0x1F;
    u0  = (u_char)(col * 8);

    /* ------------------------------------------------------------------
     * 2. Compute texture V and glyph dimensions from size mode.
     *    row = band of 32 chars the character falls into (0 = 0x20–0x3F).
     * ------------------------------------------------------------------ */
    switch (size_mode) {

    case FONT_MODE_LARGE:
        /* 8×16 — V = row * 16 (bands start at V=0, 16, 32, …)  */
        v0      = (u_char)(((char_code - 1) >> 5) << 4);
        glyph_w = 8;
        glyph_h = 16;
        break;

    case FONT_MODE_SMALL8:
        /* 8×8 — small bands start at V=48 */
        v0      = (u_char)(((char_code - 1) >> 5) * 8 + 48);
        glyph_w = 8;
        glyph_h = 8;
        break;

    case FONT_MODE_SMALL6:
        /* 6×8 — bands start at V=72 */
        v0      = (u_char)(((char_code - 1) >> 5) * 8 + 72);
        glyph_w = 6;
        glyph_h = 8;
        break;

    case FONT_MODE_SMALL4:
        /* 4×8 — bands start at V=96, draw position shifted down 4 px */
        v0      = (u_char)(((char_code - 1) >> 5) * 8 + 96);
        glyph_w = 4;
        glyph_h = 8;
        y      += 4;
        break;

    default:
        return;
    }

    /* ------------------------------------------------------------------
     * 3. Look up per-glyph position adjustments from the metrics table.
     *    Effective screen pos = input_pos + (adj - ref).
     * ------------------------------------------------------------------ */
    font_style     = zone_to_style[ctx->zone];
    metrics_offset = font_style * FONT_METRICS_STRIDE_STYLE
                   + (int)font_variant * FONT_METRICS_STRIDE_VARIANT;
    metrics = (FontMetrics *)((u_char *)FONT_METRICS_BASE + metrics_offset);

    /* ------------------------------------------------------------------
     * 4. Allocate a SPRT_VAR from the primitive buffer (20 bytes / 5 u_longs).
     * ------------------------------------------------------------------ */
    prim      = (SPRT_VAR *)prim_ptr;
    prim_ptr += 5;

    /* Primitive code and block length */
    prim->code = GP0_TEXRECT;
    setlen(prim, 4);

    /* ------------------------------------------------------------------
     * 5. Fill screen position, UV, CLUT and dimensions.
     * ------------------------------------------------------------------ */
    prim->x = metrics->adj_x + (x - metrics->ref_x);
    prim->y = metrics->adj_y + (y - metrics->ref_y);

    prim->u    = u0;
    prim->v    = v0;
    prim->clut = (palette == 0) ? FONT_CBA_MENU : FONT_CBA_HUD;
    prim->w    = glyph_w;
    prim->h    = glyph_h;

    /* ------------------------------------------------------------------
     * 6. Colour modulation.
     *    render_mode 0x0015 → fixed half-brightness (displays as white
     *    against the neutral-grey PSX palette).
     *    Otherwise → GTE normal-colour-colour (NCCS) for dynamic tinting.
     * ------------------------------------------------------------------ */
    if (ctx->render_mode == 0x0015) {
        prim->r = prim->g = prim->b = 0x80;
    } else {
        gte_ldv0(&gfx_normal);
        gte_ldrgb(&gfx_color_in);
        gte_nccs();
        gte_strgb((CVECTOR *)&prim->r);
    }

    /* ------------------------------------------------------------------
     * 7. Link the primitive into the ordering table at z_depth.
     *    ctx->ot[font_variant] is the base of an OT array;
     *    z_depth selects the bucket.
     * ------------------------------------------------------------------ */
    ot_entry = ctx->ot[font_variant] + ctx->z_depth;
    AddPrim(ot_entry, prim);

    /* ------------------------------------------------------------------
     * 8. Append a DR_MODE primitive to switch the GPU to the font tpage.
     * ------------------------------------------------------------------ */
    set_font_drawmode(ctx);
}

/*
 * set_font_drawmode — appends a DR_MODE (draw-mode change) primitive that
 * sets tpage = FONT_TPAGE (0x0007) so subsequent GPU rectangles sample the
 * correct texture page.  The primitive is linked into the same OT bucket
 * as the glyph sprite.
 */
static void set_font_drawmode(FontCtx *ctx)
{
    DR_MODE *dr = (DR_MODE *)prim_ptr;
    prim_ptr   += 3;            /* DR_MODE = 12 bytes = 3 u_longs, ilen=2   */

    SetDrawMode(dr,
                0,              /* dfe: draw-to-display-area disabled       */
                0,              /* dtd: dither disabled                     */
                FONT_TPAGE,     /* tpage 0x0007 = 4bpp font at VRAM (448,0) */
                NULL);          /* no texture window                        */

    AddPrim(ctx->ot[font_variant] + ctx->z_depth, dr);
}
