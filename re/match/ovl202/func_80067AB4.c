#include "ac1/ac1.h"

typedef struct {
    u8 unk0[4];
    SVECTOR c;                  /* +0x04 centre (pad = off flag) */
    SVECTOR a;                  /* +0x0C */
    SVECTOR b;                  /* +0x14 */
    SVECTOR d;                  /* +0x1C */
    s32 w;                      /* +0x24 */
    s32 unk28;
    s16 r;                      /* +0x2C */
    s16 unk2e;
} Sphere48;

typedef struct {
    Sphere48 s[16];
    s32 unk300;
    s32 n;                      /* +0x304 (0x801AEB30) */
} SphereSet;

extern SphereSet D_801AE82C;

/* bit mask of the spheres whose box overlaps the box (pos, h, v) */
s32 func_80067AB4(SVECTOR *pos, s32 h, s32 v)
{
    Sphere48 *p;
    s32 i;
    s32 mask;
    s32 n;

    mask = 0;
    n = D_801AE82C.n;
    p = &D_801AE82C.s[n];
    i = n;
    while (--i != -1) {
        p--;
        mask <<= 1;
        if (p->c.pad == 0) {
            if (pos->vx - h <= p->c.vx + p->r && pos->vx + h >= p->c.vx - p->r
                && pos->vy - v <= p->c.vy + p->r && pos->vy >= p->c.vy - p->r
                && pos->vz - h <= p->c.vz + p->r && pos->vz + h >= p->c.vz - p->r) {
                mask |= 1;
            }
        }
    }
    return mask;
}
