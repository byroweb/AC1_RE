#include "ac1/ac1.h"
#include "ac1/prim2d.h"

extern DrawCtx D_801A5F24;
extern u16 D_80041210;          /* radar range */
extern s32 D_800411A4;          /* radar range squared */
extern void LoadAverageCol(u8 *v0, u8 *v1, long p0, long p1, u8 *v2);
extern void func_8004FEAC(s32, s32, s32, s32, s16 *);

void func_800501DC(Projectile *pr, u8 *above, u8 *base, u8 *below, s32 c, s32 s) {
    SVECTOR unused[1];
    LINE_F3 *p;
    s32 dx;
    s32 dz;
    s32 dy;

    dx = g_ac_entities[0].pos.x - pr->x;
    if (dx > D_80041210 || dx < -D_80041210) {
        return;
    }
    dz = g_ac_entities[0].pos.z - pr->z;
    if (dz > D_80041210 || dz < -D_80041210) {
        return;
    }
    if (dx * dx + dz * dz > D_800411A4) {
        return;
    }
    p = (LINE_F3 *)D_801A5F24.buf->cur;
    D_801A5F24.buf->cur = (u8 *)(p + 1);
    if (D_801A5F24.buf->cur > D_801A5F24.buf->end) {
        return;
    }
    dy = g_ac_entities[0].pos.y - pr->y;
    func_8004FEAC(dx, dz, c, s, &p->x0);
    if (dy >= 0) {
        if (dy > 4096) {
            dy = 4096;
        }
        LoadAverageCol(base, above, 4096 - dy, dy, &p->r0);
        p->x0 += 282;
        p->y0 += 79;
        p->x1 = p->x0 - 1;
        p->y1 = p->y0 + 1;
        p->x2 = p->x0 + 1;
        p->y2 = p->y0 + 1;
    } else {
        dy = -dy;
        if (dy > 4096) {
            dy = 4096;
        }
        LoadAverageCol(base, below, 4096 - dy, dy, &p->r0);
        p->x0 += 282;
        p->y0 += 81;
        p->x1 = p->x0 - 1;
        p->y1 = p->y0 - 1;
        p->x2 = p->x0 + 1;
        p->y2 = p->y0 - 1;
    }
    setLineF3(p);
    addPrim(D_801A5F24.ot + 10, p);
}
