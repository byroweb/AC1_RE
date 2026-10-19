#include "ac1_types.h"
/* ovl202 0x80073714  size 496  score 8.645  2026-10-18
 * Pick the nearest enemy unit inside range/4 of `self` (16 slots of 0x170
 * bytes at D_801A26B8), weighting the distance by how many allies already
 * target it.  Stores the pick in self->target, returns its distance * 4 or -1. */

typedef struct Func80073714Unit {
    s32 active;                         /* +0x00 */
    u8 pad4[4];
    s16 x;                              /* +0x08 */
    u8 padA[2];
    s16 z;                              /* +0x0C */
    u8 padE[0x36 - 0xE];
    u16 team;                           /* +0x36 */
    u8 pad38[0x48 - 0x38];
    struct Func80073714Unit *target;    /* +0x48 */
    u8 pad4C[0x170 - 0x4C];
} Func80073714Unit;

extern Func80073714Unit D_801A26B8[16];
extern u32 D_8019F524;
extern s32 func_80027324(s32 v);

s32 func_80073714(Func80073714Unit *self, s32 range)
{
    Func80073714Unit *o;
    Func80073714Unit *u;
    Func80073714Unit *best_unit;
    s32 i;
    s32 j;
    s32 team;
    s32 dx;
    s32 dz;
    s32 dist;
    s32 score;
    s32 best_score;
    s32 best_dist;
    s32 count;
    s32 weight;

    o = D_801A26B8;
    best_unit = 0;
    i = 16;
    best_dist = 10000;
    best_score = 0x7FFFFFFF;
    range >>= 2;
    team = self->team & 7;
    for (; i != 0; o++, i--) {
        if (o->active == 0) {
            continue;
        }
        if ((o->team & 7) == team) {
            continue;
        }
        if (o == D_801A26B8 && (D_8019F524 & 0x400)) {
            continue;
        }
        dx = (o->x - self->x) >> 1;
        dz = (o->z - self->z) >> 1;
        dist = func_80027324(dx * dx + dz * dz);
        if (dist >= range) {
            continue;
        }
        count = 0;
        weight = 1;
        for (j = 16, u = D_801A26B8; j != 0; u++, j--) {
            if (u->active != 0 && (u->team & 7) == team) {
                count++;
                if (u != self && u->target == o) {
                    weight++;
                }
            }
        }
        if (count >= 2) {
            score = dist * weight / count;
        } else {
            score = dist;
        }
        if (score < best_score) {
            best_score = score;
            best_dist = dist;
            best_unit = o;
        }
    }
    self->target = best_unit;
    if (best_unit == 0) {
        return -1;
    }
    return best_dist * 4;
}
