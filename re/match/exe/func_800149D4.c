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

s32 func_800149D4(SVECTOR *a, SVECTOR *b)
{
    VECTOR t;

    t.vx = b->vx - a->vx;
    t.vy = b->vy - a->vy;
    t.vz = b->vz - a->vz;
    gte_ldlvl(&t);
    gte_sqr0();
    gte_stlvnl(&t);
    return SquareRoot0(t.vx + t.vy + t.vz);
}
