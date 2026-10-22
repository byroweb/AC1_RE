#include "ac1/ac1.h"
#include "ac1/prim2d.h"

typedef struct Body {
    u8 unk00[8];
    s16 x;                      /* +0x08 */
    s16 y;                      /* +0x0A */
    s16 z;                      /* +0x0C */
} Body;
typedef struct Pilot {
    u8 unk000[0x204];
    s32 range2;                 /* +0x204 radar range squared */
    u8 unk208[0x68];
    u16 range;                  /* +0x270 radar range */
    u8 unk272[0x122];
    Body *body;                 /* +0x394 */
} Pilot;
extern Pilot *D_80039D18;
extern DrawCtx D_8019D95C;
extern void LoadAverageCol(u8 *v0, u8 *v1, long p0, long p1, u8 *v2);
void func_800507B8(s32 x, s32 z, s32 a, s32 b, s16 *out);

void func_80050AF0(SVECTOR *pr, u8 *above, u8 *base, u8 *below, s32 c, s32 s) {
    SVECTOR unused[1];
    LINE_F3 *p;
    s32 dx;
    s32 dz;
    s32 dy;

    dx = D_80039D18->body->x - pr->vx;
    if (dx > D_80039D18->range || dx < -D_80039D18->range) {
        return;
    }
    dz = D_80039D18->body->z - pr->vz;
    if (dz > D_80039D18->range || dz < -D_80039D18->range) {
        return;
    }
    if (dx * dx + dz * dz > D_80039D18->range2) {
        return;
    }
    p = (LINE_F3 *)D_8019D95C.buf->cur;
    D_8019D95C.buf->cur = (u8 *)(p + 1);
    if (D_8019D95C.buf->cur > D_8019D95C.buf->end) {
        return;
    }
    dy = D_80039D18->body->y - pr->vy;
    func_800507B8(dx, dz, c, s, &p->x0);
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
    addPrim(D_8019D95C.ot + 10, p);
}
