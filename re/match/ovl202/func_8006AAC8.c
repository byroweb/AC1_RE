#include "ac1/ac1.h"
#include "ac1/gte.h"
/* ovl202 func_8006AAC8 - recursive triangle subdivider (gouraud textured).
 * A triangle whose three corners all have z 0 is dropped.  At depth 0 it is
 * emitted as a POLY_GT3 into the ordering table (corners nearer than z 40
 * are dropped); otherwise the three edge midpoints are averaged (position,
 * colour, uv), projected with the GTE, and the four sub-triangles are
 * subdivided one level down in the next SubWork frame. */

void func_8006AAC8(SubWork *w, s32 depth)
{
    POLY_GT3 *p;

    if ((w->v[0]->sz | w->v[1]->sz | w->v[2]->sz) == 0) {
        return;
    }
    if (depth == 0) {
        if (w->v[0]->sz < 40 || w->v[1]->sz < 40 || w->v[2]->sz < 40) {
            return;
        }
        p = (POLY_GT3 *)RSCR->prim;
        RSCR->prim = (u8 *)(p + 1);
        *(s32 *)&p->x0 = w->v[0]->sxy;
        *(s32 *)&p->x1 = w->v[1]->sxy;
        *(s32 *)&p->x2 = w->v[2]->sxy;
        *(u32 *)&p->u0 = *(u16 *)&w->v[0]->u | RSCR->clut;
        *(u32 *)&p->u1 = *(u16 *)&w->v[1]->u | RSCR->tpage;
        *(u16 *)&p->u2 = *(u16 *)&w->v[2]->u;
        *(u32 *)&p->r0 = *(u32 *)&w->v[0]->r;
        *(u32 *)&p->r1 = *(u32 *)&w->v[1]->r;
        *(u32 *)&p->r2 = *(u32 *)&w->v[2]->r;
        setlen(p, 9);
        p->code = RSCR->code;
        addPrim(RSCR->ot + (w->v[0]->sz >> RSCR->otshift), p);
    } else {
        w->mid[0].pos.vx = (w->v[0]->pos.vx + w->v[1]->pos.vx) >> 1;
        w->mid[0].pos.vy = (w->v[0]->pos.vy + w->v[1]->pos.vy) >> 1;
        w->mid[0].pos.vz = (w->v[0]->pos.vz + w->v[1]->pos.vz) >> 1;
        gte_ldv0(&w->mid[0]);
        gte_rtps();
        w->mid[0].r = (w->v[0]->r + w->v[1]->r) >> 1;
        w->mid[0].g = (w->v[0]->g + w->v[1]->g) >> 1;
        w->mid[0].b = (w->v[0]->b + w->v[1]->b) >> 1;
        w->mid[0].u = (w->v[0]->u + w->v[1]->u) >> 1;
        w->mid[0].v = (w->v[0]->v + w->v[1]->v) >> 1;
        gte_stsxy(&w->mid[0].sxy);
        gte_stszotz(&w->mid[0].sz);

        w->mid[1].pos.vx = (w->v[1]->pos.vx + w->v[2]->pos.vx) >> 1;
        w->mid[1].pos.vy = (w->v[1]->pos.vy + w->v[2]->pos.vy) >> 1;
        w->mid[1].pos.vz = (w->v[1]->pos.vz + w->v[2]->pos.vz) >> 1;
        gte_ldv0(&w->mid[1]);
        gte_rtps();
        w->mid[1].r = (w->v[1]->r + w->v[2]->r) >> 1;
        w->mid[1].g = (w->v[1]->g + w->v[2]->g) >> 1;
        w->mid[1].b = (w->v[1]->b + w->v[2]->b) >> 1;
        w->mid[1].u = (w->v[1]->u + w->v[2]->u) >> 1;
        w->mid[1].v = (w->v[1]->v + w->v[2]->v) >> 1;
        gte_stsxy(&w->mid[1].sxy);
        gte_stszotz(&w->mid[1].sz);

        w->mid[2].pos.vx = (w->v[2]->pos.vx + w->v[0]->pos.vx) >> 1;
        w->mid[2].pos.vy = (w->v[2]->pos.vy + w->v[0]->pos.vy) >> 1;
        w->mid[2].pos.vz = (w->v[2]->pos.vz + w->v[0]->pos.vz) >> 1;
        gte_ldv0(&w->mid[2]);
        gte_rtps();
        w->mid[2].r = (w->v[2]->r + w->v[0]->r) >> 1;
        w->mid[2].g = (w->v[2]->g + w->v[0]->g) >> 1;
        w->mid[2].b = (w->v[2]->b + w->v[0]->b) >> 1;
        w->mid[2].u = (w->v[2]->u + w->v[0]->u) >> 1;
        w->mid[2].v = (w->v[2]->v + w->v[0]->v) >> 1;
        gte_stsxy(&w->mid[2].sxy);
        gte_stszotz(&w->mid[2].sz);

        depth--;
        w[1].v[0] = &w->mid[0];
        w[1].v[1] = &w->mid[2];
        w[1].v[2] = w->v[0];
        func_8006AAC8(&w[1], depth);
        w[1].v[1] = w->v[1];
        w[1].v[2] = &w->mid[1];
        func_8006AAC8(&w[1], depth);
        w[1].v[0] = w->v[2];
        w[1].v[1] = &w->mid[2];
        func_8006AAC8(&w[1], depth);
        w[1].v[0] = &w->mid[2];
        w[1].v[1] = &w->mid[0];
        func_8006AAC8(&w[1], depth);
    }
}
