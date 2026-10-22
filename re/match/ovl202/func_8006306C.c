#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_cmd_4B08041B() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B08041B")

void func_8006306C(SVECTOR *v, void *rgb)
{
    gte_ldv0(v);
    gte_ldrgb(rgb);
    gte_cmd_4B08041B();
}
