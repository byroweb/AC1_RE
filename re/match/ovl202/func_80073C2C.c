#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x6C];
    u16 h6c;            /* +0x6C height */
} AcH;

extern u32 D_801A5DD8[];
extern int rand(void);
extern long SquareRoot0(long a);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);

s32 func_80073C2C(AcEntity *t, SVECTOR *from, s16 yaw, s32 pitch, u16 mask, s32 team, s32 range,
                  s16 xmin, s16 xmax, s16 ymin, s16 ymax, TargetSlot *slot)
{
    SVECTOR ang;
    s32 dx;
    s32 dy;
    s32 dz;

    if (t != 0 && t->def != 0 && (t->faction & 7) != (team & 7)) {
        if (!(D_801A5DD8[t->section >> 5] & (1 << (t->section & 31)))) {
            return 0;
        }
        if (t->ignore_mask & mask) {
            if (rand() < 16384) {
                return 0;
            }
        }
        slot->proj[0] = t->pos.x;
        slot->proj[2] = t->pos.z;
        slot->proj[1] = t->pos.y - (((AcH *)t)->h6c >> 1);
        dx = slot->proj[0] - from->vx;
        dy = slot->proj[1] - from->vy;
        dz = slot->proj[2] - from->vz;
        if (dx > range || dx < -range) {
            return 0;
        }
        if (dy > range || dy < -range) {
            return 0;
        }
        if (dz > range || dz < -range) {
            return 0;
        }
        if (dx * dx + dz * dz > range * range) {
            return 0;
        }
        slot->target = t;
        slot->lock = SquareRoot0(dx * dx + dz * dz + dy * dy);
        slot->flags = 1;
        func_80014AE4(dx, dy, dz, &ang);
        ang.vx = (ang.vx - pitch) & 0xFFF;
        if (ang.vx >= 2048) {
            ang.vx -= 4096;
        }
        if (ang.vx >= xmin && ang.vx <= xmax) {
            ang.vy = (ang.vy - yaw) & 0xFFF;
            if (ang.vy >= 2048) {
                ang.vy -= 4096;
            }
            if (ang.vy >= ymin && ang.vy <= ymax) {
                slot->flags |= 2;
            }
        }
        *(SVECTOR *)slot->aim = ang;
        return 1;
    }
    return 0;
}
