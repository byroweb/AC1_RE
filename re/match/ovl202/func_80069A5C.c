#include "ac1/ac1.h"
#include "ac1/gte.h"
/* ovl202 func_80069A5C - recursive triangle subdivider, flat textured: the
 * POLY_FT3 sibling of func_8006AAC8 (one colour from the scratchpad, only the
 * uv is averaged at the midpoints). */

void func_80069A5C(SubWork *w, s32 depth)
{
    POLY_FT3 *p;

    if ((w->v[0]->sz | w->v[1]->sz | w->v[2]->sz) == 0) {
        return;
    }
    if (depth == 0) {
        if (w->v[0]->sz < 40 || w->v[1]->sz < 40 || w->v[2]->sz < 40) {
            return;
        }
        p = (POLY_FT3 *)RSCR->prim;
        RSCR->prim = (u8 *)(p + 1);
        *(s32 *)&p->x0 = w->v[0]->sxy;
        *(s32 *)&p->x1 = w->v[1]->sxy;
        *(s32 *)&p->x2 = w->v[2]->sxy;
        *(u32 *)&p->u0 = *(u16 *)&w->v[0]->u | RSCR->clut;
        *(u32 *)&p->u1 = *(u16 *)&w->v[1]->u | RSCR->tpage;
        *(u16 *)&p->u2 = *(u16 *)&w->v[2]->u;
        *(u32 *)&p->r0 = *(u32 *)&RSCR->r;
        setlen(p, 7);
        p->code = RSCR->code_ft;
        addPrim(RSCR->ot + (w->v[0]->sz >> RSCR->otshift), p);
    } else {
        w->mid[0].pos.vx = (w->v[0]->pos.vx + w->v[1]->pos.vx) >> 1;
        w->mid[0].pos.vy = (w->v[0]->pos.vy + w->v[1]->pos.vy) >> 1;
        w->mid[0].pos.vz = (w->v[0]->pos.vz + w->v[1]->pos.vz) >> 1;
        gte_ldv0(&w->mid[0]);
        gte_rtps();
        w->mid[0].u = (w->v[0]->u + w->v[1]->u) >> 1;
        w->mid[0].v = (w->v[0]->v + w->v[1]->v) >> 1;
        gte_stsxy(&w->mid[0].sxy);
        gte_stszotz(&w->mid[0].sz);

        w->mid[1].pos.vx = (w->v[1]->pos.vx + w->v[2]->pos.vx) >> 1;
        w->mid[1].pos.vy = (w->v[1]->pos.vy + w->v[2]->pos.vy) >> 1;
        w->mid[1].pos.vz = (w->v[1]->pos.vz + w->v[2]->pos.vz) >> 1;
        gte_ldv0(&w->mid[1]);
        gte_rtps();
        w->mid[1].u = (w->v[1]->u + w->v[2]->u) >> 1;
        w->mid[1].v = (w->v[1]->v + w->v[2]->v) >> 1;
        gte_stsxy(&w->mid[1].sxy);
        gte_stszotz(&w->mid[1].sz);

        w->mid[2].pos.vx = (w->v[2]->pos.vx + w->v[0]->pos.vx) >> 1;
        w->mid[2].pos.vy = (w->v[2]->pos.vy + w->v[0]->pos.vy) >> 1;
        w->mid[2].pos.vz = (w->v[2]->pos.vz + w->v[0]->pos.vz) >> 1;
        gte_ldv0(&w->mid[2]);
        gte_rtps();
        w->mid[2].u = (w->v[2]->u + w->v[0]->u) >> 1;
        w->mid[2].v = (w->v[2]->v + w->v[0]->v) >> 1;
        gte_stsxy(&w->mid[2].sxy);
        gte_stszotz(&w->mid[2].sz);

        depth--;
        w[1].v[0] = &w->mid[0];
        w[1].v[1] = &w->mid[2];
        w[1].v[2] = w->v[0];
        func_80069A5C(&w[1], depth);
        w[1].v[1] = w->v[1];
        w[1].v[2] = &w->mid[1];
        func_80069A5C(&w[1], depth);
        w[1].v[0] = w->v[2];
        w[1].v[1] = &w->mid[2];
        func_80069A5C(&w[1], depth);
        w[1].v[0] = &w->mid[2];
        w[1].v[1] = &w->mid[0];
        func_80069A5C(&w[1], depth);
    }
}
