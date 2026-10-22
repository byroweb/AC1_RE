#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ncds_word() __asm__ volatile ("nop\n\tnop\n\t.word 0x4AE80413")

void func_8005BE04(void *rgb, s32 p)
{
    gte_ldrgb(rgb);
    gte_lddp(p);
    gte_ncds_word();
}
