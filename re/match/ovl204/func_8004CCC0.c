#include "ac1/ac1.h"

#define gte_ldlvl(r0) __asm__ volatile ( \
    "lwc2 $9, 0(%0)\n\t" \
    "lwc2 $10, 4(%0)\n\t" \
    "lwc2 $11, 8(%0)" : : "r"(r0))
#define gte_sqr0() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4AA00428")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25, 0(%0)\n\t" \
    "swc2 $26, 4(%0)\n\t" \
    "swc2 $27, 8(%0)" : : "r"(r0) : "memory")

extern long SquareRoot0(long a);

/* Distance attenuation of a positioned sound heard at ear l: 0 when out of
 * range or quieter than 30, else the volume (max 127). */
s32 func_8004CCC0(SVECTOR *p, SVECTOR *l, s32 vol, s32 range, s32 len)
{
    VECTOR *d = (VECTOR *)0x1F8003F0;
    VECTOR *sq = (VECTOR *)0x1F8003E0;
    s32 dist;

    d->vx = (p->vx - l->vx) >> 1;
    d->vy = (p->vy - l->vy) >> 1;
    d->vz = (p->vz - l->vz) >> 1;
    gte_ldlvl(d);
    gte_sqr0();
    gte_stlvnl(sq);
    dist = SquareRoot0(sq->vx + sq->vy + sq->vz) * 2;
    if (dist >= range) {
        return 0;
    }
    dist = ((len - dist) << 7) / len;
    vol = (dist * vol) >> 7;
    if (vol < 30) {
        return 0;
    }
    if (vol >= 128) {
        vol = 127;
    }
    return vol;
}
