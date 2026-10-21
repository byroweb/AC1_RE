#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ncds_word() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")

void func_8005911C(SVECTOR *v, void *rgb, s32 p)
{
    gte_ldv0(v);
    gte_ldrgb(rgb);
    gte_lddp(p);
    gte_ncds_word();
}
