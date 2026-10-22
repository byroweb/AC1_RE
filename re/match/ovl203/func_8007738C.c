/* ovl203 func_8007738C: versus pause.  When either player presses START (pad bit 0x800),
 * draw the "PAUSE" quad over the current frame, mute, park the CD, then spin on VSync +
 * the link pad exchange until START is released, pressed again and released; resume the
 * CD stream (crediting the paused time) and restore the master volume. */
#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

typedef struct {
    s32 pad_raw;        /* +0x00 */
    s32 pad_raw_prev;   /* +0x04 */
    s32 pad_cur;        /* +0x08 */
    s32 pad_prev;       /* +0x0C */
    u8 unk10[0x254 - 0x10];
    s32 flags254;       /* +0x254 */
} PadBlk;

extern PadBlk *D_80039D30;
extern PadBlk *D_80039D34;
extern u8 D_80041BBB;
extern u16 D_80041BBE;
extern volatile struct { u8 v; } D_80041BC2;
extern struct { s32 v; } D_80041BC4;
extern u8 D_800895C8;
extern struct { u16 v; } D_80194740;
extern s32 D_8019D850;
extern GfxFrame D_8019D854[2];

extern void DrawSync(s32 mode);
extern void PutDrawEnv(DRAWENV *env);
extern void DrawPrim(void *p);
extern void SsSetMVol(s32 l, s32 r);
extern s32 CdControl(s32 com, u8 *param, u8 *result);
extern s32 CdRead2(s32 mode);
extern s32 VSync(s32 mode);
extern s32 func_8001628C(void);
extern void func_80077E10(void);

void func_8007738C(void)
{
    POLY_FT4 prim;
    POLY_FT4 *p;
    GfxFrame *f;
    s32 t;

    p = &prim;
    if (!(D_80039D30->pad_cur & 0x800)) {
        if (!(D_80039D34->pad_cur & 0x800))
            return;
    } else {
        D_80039D30->flags254 &= ~0x10000;
        D_80194740.v &= ~1;
    }
    DrawSync(0);
    f = &D_8019D854[0];
    if (D_8019D850 == 0)
        f = &D_8019D854[1];
    f->draw.isbg = 0;
    PutDrawEnv(&f->draw);
    f->draw.isbg = 1;
    p->r0 = 0;
    p->g0 = 250;
    p->b0 = 0;
    p->clut = 0x7D10;
    p->tpage = 30;
    p->x0 = 121;
    p->y0 = 109;
    p->x1 = 199;
    p->y1 = 109;
    p->x2 = 121;
    p->y2 = 131;
    p->x3 = 199;
    p->y3 = 131;
    p->u0 = 49;
    p->v0 = 201;
    p->u1 = 127;
    p->v1 = 201;
    p->u2 = 49;
    p->v2 = 223;
    p->u3 = 127;
    p->v3 = 223;
    setlen(p, 9);
    setcode(p, 0x2E);
    DrawPrim(p);
    DrawSync(0);
    SsSetMVol(0, 0);
    while (D_80041BC2.v == 0xFF) {
    }
    if (D_80041BBE != 0xFFFF) {
        t = func_8001628C();
        while (CdControl(9, 0, 0) == 0) {
        }
    }
    for (;;) {
        VSync(2);
        func_80077E10();
        if (D_800895C8 != 0)
            return;
        if (!(D_80039D30->pad_cur & 0x800) && !(D_80039D34->pad_cur & 0x800))
            break;
        if (D_80039D30->pad_cur == 0x890)
            D_80039D30->flags254 |= 0x10000;
        if (D_80039D30->pad_cur == 0x860)
            D_80194740.v |= 1;
        if (D_80039D34->pad_cur == 0x890)
            D_80039D34->flags254 |= 0x10000;
    }
    do {
        VSync(2);
        func_80077E10();
        if (D_800895C8 != 0)
            return;
    } while (!(D_80039D30->pad_cur & 0x800) && !(D_80039D34->pad_cur & 0x800));
    do {
        VSync(2);
        func_80077E10();
        if (D_800895C8 != 0)
            return;
    } while ((D_80039D30->pad_cur & 0x800) || (D_80039D34->pad_cur & 0x800));
    if (D_80041BBE != 0xFFFF) {
        while (CdRead2(200) == 0) {
        }
        D_80041BC4.v += func_8001628C() - t;
    }
    SsSetMVol(D_80041BBB, D_80041BBB);
    D_80039D30->pad_cur = 0;
    D_80039D34->pad_cur = 0;
}
