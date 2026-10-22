#include "ac1/ac1.h"
#include "ac1/prim2d.h"

extern DrawCtx D_801ADCCC;
extern s32 D_800AD958;
extern s32 func_8004FE38(s32, s32, s32, s32, s32, s32 *, s32 *, s32 *, s32 *);

void func_80050344(s32 x0, s32 y0, s32 x1, s32 y1, u32 *rgb) {
    LINE_F2 *p;
    s32 off;
    s32 ax, ay, bx, by;

    if (func_8004FE38(x0, y0, x1, y1, 484, &x0, &y0, &x1, &y1) == 0) {
        return;
    }
    p = (LINE_F2 *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(p + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    *(u32 *)&p->r0 = *rgb;
    ax = x0; off = D_800AD958 + 43; ay = y0; bx = x1; by = y1;
    setLineF2(p);
    setLineF2(p);
    p->x0 = ax + off;
    p->y0 = ay + 192;
    p->x1 = bx + off;
    p->y1 = by + 192;
    addPrim(D_801ADCCC.ot + 3, p);
}
