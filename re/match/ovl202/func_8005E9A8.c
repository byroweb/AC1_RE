#include "ac1/ac1.h"

extern DrawCtx D_801A5F24;

/* textured sprite rectangle (w x h at pos) from texture area s at OT slot z */
void func_8005E9A8(DVECTOR *pos, SprInfo *s, u32 *rgb, s32 z, s32 abe, s32 w, s32 h)
{
    POLY_FT4 *p;
    u32 *ot;

    p = (POLY_FT4 *)D_801A5F24.buf->cur;
    ot = &D_801A5F24.ot[z];
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    *(u32 *)&p->r0 = *rgb;
    p->clut = s->clut;
    p->tpage = s->tpage;
    p->x0 = pos->vx;
    p->y0 = pos->vy;
    p->x1 = pos->vx + w;
    p->y1 = pos->vy;
    p->x2 = pos->vx;
    p->y2 = pos->vy + h;
    p->x3 = pos->vx + w;
    p->y3 = pos->vy + h;
    p->u0 = s->u;
    p->v0 = s->v;
    p->u1 = s->u + s->w;
    p->v1 = s->v;
    p->u2 = s->u;
    p->v2 = s->v + s->h;
    p->u3 = s->u + s->w;
    p->v3 = s->v + s->h;
    setlen(p, 9);
    setcode(p, abe | 0x2C);
    addPrim(ot, p);
}
