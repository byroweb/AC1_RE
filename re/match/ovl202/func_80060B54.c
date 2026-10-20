#include "ac1/ac1.h"
/* ovl202 func_80060B54 - emit a 2D sprite as a POLY_FT4 at xy, turned by
 * rot quarter turns (texture coordinates rotated, w/h swapped for odd
 * turns), colour *rgb, semi-transparency code bits abr, into ordering-table
 * slot z.  Nothing is drawn when the primitive buffer is full. */

extern DrawCtx D_801A5F24;

void func_80060B54(DVECTOR *xy, SprInfo *spr, s32 rot, u32 *rgb, s32 z, s32 abr)
{
    POLY_FT4 *p;
    u32 *ot;

    p = (POLY_FT4 *)D_801A5F24.buf->cur;
    ot = D_801A5F24.ot + z;
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    *(u32 *)&p->r0 = *rgb;
    p->clut = spr->clut;
    p->tpage = spr->tpage;
    switch (rot) {
    case 0:
        p->x0 = xy->vx;
        p->y0 = xy->vy;
        p->x1 = xy->vx + spr->w;
        p->y1 = xy->vy;
        p->x2 = xy->vx;
        p->y2 = xy->vy + spr->h;
        p->x3 = xy->vx + spr->w;
        p->y3 = xy->vy + spr->h;
        p->u0 = spr->u;
        p->v0 = spr->v;
        p->u1 = spr->u + spr->w;
        p->v1 = spr->v;
        p->u2 = spr->u;
        p->v2 = spr->v + spr->h;
        p->u3 = spr->u + spr->w;
        p->v3 = spr->v + spr->h;
        break;
    case 1:
        p->x0 = xy->vx;
        p->y0 = xy->vy;
        p->x1 = xy->vx + spr->h;
        p->y1 = xy->vy;
        p->x2 = xy->vx;
        p->y2 = xy->vy + spr->w;
        p->x3 = xy->vx + spr->h;
        p->y3 = xy->vy + spr->w;
        p->u2 = spr->u - 1;
        p->v2 = spr->v;
        p->u0 = spr->u + spr->w - 1;
        p->v0 = spr->v;
        p->u3 = spr->u - 1;
        p->v3 = spr->v + spr->h;
        p->u1 = spr->u + spr->w - 1;
        p->v1 = spr->v + spr->h;
        break;
    case 2:
        p->x0 = xy->vx;
        p->y0 = xy->vy;
        p->x1 = xy->vx + spr->w;
        p->y1 = xy->vy;
        p->x2 = xy->vx;
        p->y2 = xy->vy + spr->h;
        p->x3 = xy->vx + spr->w;
        p->y3 = xy->vy + spr->h;
        p->u3 = spr->u - 1;
        p->v3 = spr->v - 1;
        p->u2 = spr->u + spr->w - 1;
        p->v2 = spr->v - 1;
        p->u1 = spr->u - 1;
        p->v1 = spr->v + spr->h - 1;
        p->u0 = spr->u + spr->w - 1;
        p->v0 = spr->v + spr->h - 1;
        break;
    case 3:
        p->x0 = xy->vx;
        p->y0 = xy->vy;
        p->x1 = xy->vx + spr->h;
        p->y1 = xy->vy;
        p->x2 = xy->vx;
        p->y2 = xy->vy + spr->w;
        p->x3 = xy->vx + spr->h;
        p->y3 = xy->vy + spr->w;
        p->u1 = spr->u;
        p->v1 = spr->v - 1;
        p->u3 = spr->u + spr->w;
        p->v3 = spr->v - 1;
        p->u0 = spr->u;
        p->v0 = spr->v + spr->h - 1;
        p->u2 = spr->u + spr->w;
        p->v2 = spr->v + spr->h - 1;
        break;
    }
    setlen(p, 9);
    p->code = abr | 0x2C;
    addPrim(ot, p);
}
