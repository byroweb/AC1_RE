#include "ac1/ac1.h"

/* ovl204: draw a string with the 2D glyph font; backslash-digit picks a colour. Returns its width. */
typedef struct Glyph {          /* 12 */
    u16 clut;
    u16 tpage;
    u8 u, v, w, h;
    u8 pad[4];
} Glyph;

extern DrawCtx D_801ADCCC;
extern u32 D_800894C0[];        /* colours */
extern s8 D_800894E0[];         /* char -> glyph index */
extern Glyph D_8008A884[];

s32 func_8005145C(s32 x, s32 y, s32 ot, s32 font, s32 colour, u8 *s)
{
    u32 *col;
    s32 n;
    s32 c;
    s32 g;
    POLY_FT4 *p;
    Glyph *gl;

    n = 0;
    col = &D_800894C0[colour];
    c = *s++;
    while (c != 0) {
        if ((u8)c == 0x5C) {
            c = *s++;
            if ((u32)(c - 0x30) < 10) {
                col = &D_800894C0[c - 0x30];
            }
        } else if ((u32)(c - 0x21) < 95) {
            g = D_800894E0[(u8)c];
            if (g != -1) {
                g += font;
                p = (POLY_FT4 *)D_801ADCCC.buf->cur;
                D_801ADCCC.buf->cur = (u8 *)(p + 1);
                if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
                    return n;
                }
                gl = &D_8008A884[g];
                *(u32 *)&p->r0 = *col;
                p->clut = gl->clut;
                p->tpage = gl->tpage;
                p->x0 = x;
                p->y0 = y;
                p->x1 = gl->w + x;
                p->y1 = y;
                p->x2 = x;
                p->y2 = gl->h + y;
                p->x3 = gl->w + x;
                p->y3 = gl->h + y;
                p->u0 = gl->u;
                p->v0 = gl->v;
                p->u1 = gl->u + gl->w;
                p->v1 = gl->v;
                p->u2 = gl->u;
                p->v2 = gl->v + gl->h;
                p->u3 = gl->u + gl->w;
                p->v3 = gl->v + gl->h;
                setlen(p, 9);
                setcode(p, 0x2E);
                addPrim(D_801ADCCC.ot + ot, p);
                x += 1 + gl->w;
                n += 1 + gl->w;
            }
        } else if ((u8)c == 0x20) {
            x += 1 + D_8008A884[font + 10].w;
            n += 1 + D_8008A884[font + 10].w;
        }
        c = *s++;
    }
    return n;
}
