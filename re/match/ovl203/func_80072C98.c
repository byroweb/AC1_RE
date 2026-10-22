/* ovl203 func_80072C98: versus target scan.  For both versus ACs not on `team` and not
 * masked by `mask | 0x10`, inside a cube of +-range and a horizontal circle of `range`
 * around `p`, fill a TargetSlot (target, distance, centre point, flags 1; |2 when the
 * direction lies in the pitch window [pmin,pmax] and yaw window [ymin,ymax]).
 * Returns the number of slots written. */
#include "ac1/ac1.h"

typedef struct {
    void *def;          /* +0x00 */
    u8 pad4[4];
    s16 x, y, z;        /* +0x08 */
    u8 pad0e[0x36 - 0x0E];
    u16 faction;        /* +0x36 */
    u16 ignore_mask;    /* +0x38 */
    u8 pad3a[0x6C - 0x3A];
    u16 h;              /* +0x6C */
    u8 pad6e[0x170 - 0x6E];
} V72Ent;

extern V72Ent D_8019B550[];
extern s32 SquareRoot0(s32 v);
extern void func_80014AE4(s32 dx, s32 dy, s32 dz, SVECTOR *ang);

s32 func_80072C98(SVECTOR *p, u16 yaw, u16 pitch, u16 mask, s32 team, s32 range,
                  s16 pmin, s16 pmax, s16 ymin, s16 ymax, TargetSlot *slot)
{
    V72Ent *e;
    s32 i;
    s32 n;
    s32 dx, dy, dz;
    s32 d2;
    s32 r2;
    s32 nr;
    s32 t;
    SVECTOR ang;

    i = 2;
    e = D_8019B550;
    n = 0;
    mask |= 0x10;
    nr = -range;
    team &= 7;
    r2 = range * range;
    for (; i != 0; i--, e++) {
        if (e->def == 0)
            continue;
        if ((e->faction & 7) == team)
            continue;
        if (e->ignore_mask & mask)
            continue;
        slot->proj[0] = e->x;
        slot->proj[2] = e->z;
        slot->proj[1] = e->y - (e->h >> 1);
        dx = slot->proj[0] - p->vx;
        dy = slot->proj[1] - p->vy;
        dz = slot->proj[2] - p->vz;
        if (dx > range || dx < nr || dy > range || dy < nr || dz > range || dz < nr)
            continue;
        d2 = dx * dx + dz * dz;
        if (d2 > r2)
            continue;
        slot->target = (AcEntity *)e;
        slot->lock = SquareRoot0(d2 + dy * dy);
        slot->flags = 1;
        func_80014AE4(dx, dy, dz, &ang);
        t = (u16)((u16)ang.vx - pitch) & 0xFFF;
        ang.vx = t;
        if (t >= 0x800)
            ang.vx = t - 0x1000;
        if (ang.vx >= pmin && pmax >= ang.vx) {
            t = (u16)((u16)ang.vy - yaw) & 0xFFF;
            ang.vy = t;
            if (t >= 0x800)
                ang.vy = t - 0x1000;
            if (ang.vy >= ymin && ymax >= ang.vy)
                slot->flags |= 2;
        }
        *(SVECTOR *)slot->aim = ang;
        slot++;
        n++;
    }
    return n;
}
