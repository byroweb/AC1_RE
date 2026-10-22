#include "ac1/ac1.h"

typedef struct {
    s32 count;
    POLY_FT4 prim;
} LoadingBar;
extern LoadingBar D_80049FA0;

void func_80012048(void)
{
    POLY_FT4 *p;
    s32 c;

    if (!(D_80049FA0.count & 0xC0)) {
        DrawSync(0);
        c = rsin(D_80049FA0.count) / 96 + 100;
        p = &D_80049FA0.prim;
        p->clut = 0xC00;
        p->tpage = 0x20;
        p->x0 = 32, p->y0 = 96, p->x1 = 287, p->y1 = 96, p->x2 = 32, p->y2 = 144, p->x3 = 287, p->y3 = 144;
        p->u0 = 0, p->v0 = 0, p->u1 = 255, p->v1 = 0, p->u2 = 0, p->v2 = 48, p->u3 = 255, p->v3 = 48;
        ((P_TAG *)p)->len = 9, ((P_TAG *)p)->code = 0x2C;
        p->r0 = c, p->g0 = c, p->b0 = c;
        DrawPrim(p);
    }
    D_80049FA0.count += 0x40;
}
