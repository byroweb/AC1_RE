#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_stsv(r0) __asm__ volatile ("mfc2 $12, $9\n\tmfc2 $13, $10\n\tmfc2 $14, $11\n\tsh $12, 0(%0)\n\tsh $13, 2(%0)\n\tsh $14, 4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

/* store IR1..IR3 as a short vector */
void func_80059178(SVECTOR *out)
{
    gte_stsv(out);
}
