#include "ac1/ac1.h"
/* ovl202 0x80051150 - print an ASCII string as POLY_FT4 glyphs at x,y into OT slot z.
 * "\N" (N digit) switches the colour; returns the pixel width drawn. */

typedef struct {
    SprInfo spr;
    u8 pad[4];
} Glyph;

extern DrawCtx D_801A5F24;
extern u32 D_8008DA1C[];
extern s8 D_8008DA3C[];
extern Glyph D_8008ED68[];

s32 func_80051150(s32 x, s32 y, s32 z, s32 font, s32 color, u8 *s)
{
    u32 *rgb;
    s32 w;
    s32 c;
    s32 k;
    s32 d;
    u8 e;
    s32 g;
    s32 cw;
    POLY_FT4 *p;
    SprInfo *spr;

    w = 0;
    rgb = &D_8008DA1C[color];
    while ((c = *s++) != 0) {
        k = c & 0xFF;
        if (k == 0x5C) {
            c = *s++;
            if ((u32)(c - '0') < 10) {
                rgb = &D_8008DA1C[c - '0'];
            }
        } else if ((u32)(c - '!') < 0x5F) {
            g = D_8008DA3C[k];
            if (g != -1) {
                g += font;
                p = (POLY_FT4 *)D_801A5F24.buf->cur;
                D_801A5F24.buf->cur = (u8 *)(p + 1);
                if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
                    return w;
                }
                spr = &D_8008ED68[g].spr;
                *(u32 *)&p->r0 = *rgb;
                p->clut = spr->clut;
                p->tpage = spr->tpage;
                p->x0 = x;
                p->y0 = y;
                p->x1 = spr->w + x;
                p->y1 = y;
                p->x2 = x;
                p->y2 = spr->h + y;
                p->x3 = spr->w + x;
                p->y3 = spr->h + y;
                p->u0 = spr->u;
                p->v0 = spr->v;
                p->u1 = spr->u + spr->w;
                p->v1 = spr->v;
                p->u2 = spr->u;
                p->v2 = spr->v + spr->h;
                p->u3 = spr->u + spr->w;
                p->v3 = spr->v + spr->h;
                setlen(p, 9);
                p->code = 0x2C;
                addPrim(D_801A5F24.ot + z, p);
                x += spr->w + 1;
                w += spr->w + 1;
            }
        } else if (k == 0x20) {
            cw = D_8008ED68[font + 10].spr.w;
            x += cw + 1;
            w += cw + 1;
        }
    }
    return w;
}
