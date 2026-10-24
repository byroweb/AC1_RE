#include "ac1/ac1.h"
/* ovl202 0x80052B28 - type out up to n characters of a message window's text: ASCII
 * control codes (newline, Cn colour, Wn wait, F next box, Kn icon, Tn/Hn sound) and
 * 6 px (single byte) / 12 px (SJIS-like two byte) glyphs as POLY_FT4 in OT slot 13.
 * Returns 1 at the end of the text, -1 when a wait ran out, else 0. */

typedef struct {
    u8 pad00[8];
    s16 x;                      /* +0x08 */
    s16 y;                      /* +0x0A */
    u8 mode;                    /* +0x0C bit 2: always draw */
    u8 pad0d[2];
    u8 col;                     /* +0x0F */
    u8 *text;                   /* +0x10 */
} MsgWin;

typedef struct {
    u8 c[12];
} IconSpr;

extern u8 D_80039D10;
extern IconSpr D_8008DB98[];
extern u32 D_801ED854[];
extern DrawCtx D_801A5F24;
extern s32 func_80052350(u8 **pp);
extern void func_8005277C(s32 a0, s32 a1, u8 *p);
extern void func_80060B54(DVECTOR *xy, IconSpr *spr, s32 rot, u32 *rgb, s32 z, s32 abr);
extern void func_8004D424(s32 id, s32 a, s32 b, s32 c, s32 d);

s32 func_80052B28(MsgWin *w, s32 n)
{
    s32 x;
    s32 y;
    u32 col;
    u32 *ot;
    u8 *q;
    u32 c;
    u8 c0;
    u8 *p;
    POLY_FT4 *pr;
    s32 u;
    s32 v;
    s32 k;

    x = w->x;
    y = w->y;
    p = w->text;
    col = w->col;
    ot = D_801A5F24.ot + 13;
top:
    while (n > 0) {
        c0 = *p;
        q = p;
        c = c0;
        p = q + 1;
        if (c == 0) {
            return 1;
        }
        if (c < 0x80) {
            switch (c) {
            case '\n':
                x = w->x;
                y += 12;
                break;
            case 'C':
            case 'c':
                col = func_80052350(&p);
                break;
            case 'W':
            case 'w':
                n -= func_80052350(&p);
                if (n > 0) {
                    goto top;
                }
                return -1;
            case 'F':
            case 'f':
                func_8005277C(w->mode, -1, p);
                return 0;
            case 'K':
            case 'k':
            {
                DVECTOR pos;
                pos.vx = x;
                pos.vy = y;
                func_80060B54(&pos, &D_8008DB98[func_80052350(&p)], 0, &D_801ED854[col], 13, 0);
            }
            case ' ':
                x += 6;
                break;
            case 'T':
            case 't':
                if (n == 1) {
                    func_8004D424(func_80052350(&p), 120, 120, 0, 1);
                }
                break;
            case 'H':
            case 'h':
                if (n == 1) {
                    func_8004D424(func_80052350(&p) + 64, 120, 120, 0, 2);
                }
                break;
            }
        } else {
            n--;
            if (D_80039D10 != 0 || (w->mode & 4)) {
                pr = (POLY_FT4 *)D_801A5F24.buf->cur;
                D_801A5F24.buf->cur = (u8 *)(pr + 1);
                if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
                    break;
                }
                *(u32 *)&pr->r0 = D_801ED854[col];
                pr->tpage = 29;
                if (c < 0xF0) {
                    s32 u, v;
                    pr->x0 = pr->x2 = x;
                    pr->y0 = pr->y1 = y;
                    pr->clut = 0x7D10;
                    pr->x1 = pr->x3 = x + 5;
                    pr->y2 = pr->y3 = y + 12;
                    c -= 128;
                    u = (s32)c % 42 * 6;
                    x += 6;
                    v = (s32)c / 42 * 12;
                    pr->u0 = u;
                    pr->v0 = v;
                    pr->v1 = v;
                    pr->u1 = u + 5;
                    pr->u2 = u;
                    pr->v2 = v + 12;
                    pr->u3 = u + 5;
                    pr->v3 = v + 12;
                } else {
                    s32 u, v;
                    pr->x0 = pr->x2 = x;
                    pr->y0 = pr->y1 = y;
                    pr->x1 = pr->x3 = x + 11;
                    pr->y2 = pr->y3 = y + 12;
                    p = q + 2;
                    c = ((c0 & 0xF) << 7) + q[1] - 2;
                    x += 12;
                    pr->clut = ((s32)c / 420 << 6) + 0x7D10;
                    c = (s32)c % 420;
                    u = (s32)c % 21 * 12;
                    v = (s32)c / 21 * 12;
                    pr->u0 = u + 1;
                    pr->v0 = v;
                    pr->v1 = v;
                    pr->u1 = u + 12;
                    pr->u2 = u + 1;
                    pr->v2 = v + 12;
                    pr->u3 = u + 12;
                    pr->v3 = v + 12;
                }
                setlen(pr, 9);
                pr->code = 0x2C;
                addPrim(ot, pr);
            }
        }
    }
    return 0;
}
