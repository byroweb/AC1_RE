#include "ac1/ac1.h"
#include "ac1/prim2d.h"

extern DrawCtx D_801A5F24;
extern s32 func_8004FA44(s32, s32, s32, s32, s32, s32 *, s32 *, s32 *, s32 *);

void func_8004FF48(s32 x0, s32 y0, s32 x1, s32 y1, u32 *rgb) {
    LINE_F2 *p;

    if (func_8004FA44(x0, y0, x1, y1, 484, &x0, &y0, &x1, &y1) == 0) {
        return;
    }
    p = (LINE_F2 *)D_801A5F24.buf->cur;
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    *(u32 *)&p->r0 = *rgb;
    p->x0 = x0 + 282;
    p->y0 = y0 + 80;
    p->x1 = x1 + 282;
    p->y1 = y1 + 80;
    setLineF2(p);
    setLineF2(p);
    addPrim(D_801A5F24.ot + 10, p);
}
