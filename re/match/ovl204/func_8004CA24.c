#include "ac1/ac1.h"
#include "ac1/ovl204_sound.h"
#include "ac1/gte.h"

#define gte_ldlvir(r0) __asm__ volatile (     "lwc2 $9, 0(%0)\n\t"     "lwc2 $10, 4(%0)\n\t"     "lwc2 $11, 8(%0)" : : "r"(r0))

#define gte_sqr12() __asm__ volatile (     "nop\n\t"     "nop\n\t"     ".word 0x4AA00428")

#define gte_stlvnl(r0) __asm__ volatile (     "swc2 $25, 0(%0)\n\t"     "swc2 $26, 4(%0)\n\t"     "swc2 $27, 8(%0)" : : "r"(r0) : "memory")

#define SQ ((VECTOR *)0x1F8003D0)

/* Volume of a sound at p: vol scaled down linearly with distance. */
s32 func_8004CA24(SVECTOR *p, s32 unused, s32 vol, s32 range, s32 maxd, VECTOR *d) {
    s32 dist;
    VECTOR *sq = SQ;

    d->vx = (p->vx - D_80041B90.pos.vx) >> 1;
    d->vy = (p->vy - D_80041B90.pos.vy) >> 1;
    d->vz = (p->vz - D_80041B90.pos.vz) >> 1;
    gte_ldlvir(d);
    gte_sqr12();
    gte_stlvnl(sq);
    dist = SquareRoot0(sq->vx + sq->vy + sq->vz);
    d->pad = dist;
    dist *= 2;
    if (dist >= range) {
        return 0;
    }
    return ((((maxd - dist) << 7) / maxd) * vol) >> 7;
}
