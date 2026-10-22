#include "ac1/ac1.h"
#include "ac1/prim2d.h"

/* ovl204 port of ovl202 0x800501DC: the player record is reached through D_80039D18 */
typedef struct {
    u8 pad000[0x204];
    s32 range_sq;               /* +0x204 radar range squared (ovl202 pl->range_sq) */
    u8 pad208[0x270 - 0x208];
    u16 range;                  /* +0x270 radar range (ovl202 pl->range) */
    u8 pad272[0x394 - 0x272];
    AcEntity *ent;              /* +0x394 */
} Pl4;

extern Pl4 *D_80039D18;
extern DrawCtx D_801ADCCC;
extern s32 D_800AD958;
extern void LoadAverageCol(u8 *v0, u8 *v1, long p0, long p1, u8 *v2);
extern void func_800502A4(s32, s32, s32, s32, s16 *);

void func_800505E8(Projectile *pr, u8 *above, u8 *base, u8 *below, s32 c, s32 s) {
    SVECTOR unused[1];
    LINE_F3 *p;
    s32 dx;
    s32 dz;
    s32 dy;
    Pl4 *pl = D_80039D18;

    dx = pl->ent->pos.x - pr->x;
    if (dx > pl->range || dx < -pl->range) {
        return;
    }
    dz = pl->ent->pos.z - pr->z;
    if (dz > pl->range || dz < -pl->range) {
        return;
    }
    if (dx * dx + dz * dz > pl->range_sq) {
        return;
    }
    p = (LINE_F3 *)D_801ADCCC.buf->cur;
    D_801ADCCC.buf->cur = (u8 *)(p + 1);
    if (D_801ADCCC.buf->cur > D_801ADCCC.buf->end) {
        return;
    }
    dy = pl->ent->pos.y - pr->y;
    func_800502A4(dx, dz, c, s, &p->x0);
    if (dy >= 0) {
        if (dy > 4096) {
            dy = 4096;
        }
        LoadAverageCol(base, above, 4096 - dy, dy, &p->r0);
        p->x0 += 43 + D_800AD958;
        p->y0 += 191;
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
        p->x0 += 43 + D_800AD958;
        p->y0 += 193;
        p->x1 = p->x0 - 1;
        p->y1 = p->y0 - 1;
        p->x2 = p->x0 + 1;
        p->y2 = p->y0 - 1;
    }
    setLineF3(p);
    addPrim(D_801ADCCC.ot + 3, p);
}
