#include "ac1/ac1.h"
#include "ac1/gte.h"

/* PsyQ inline_c.h shapes not in gte.h yet */
#define gte_rtv0tr() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A480012")

#define gte_stsv(r0) __asm__ volatile ( \
    "mfc2 $12, $9\n\t" \
    "mfc2 $13, $10\n\t" \
    "mfc2 $14, $11\n\t" \
    "sh $12, 0(%0)\n\t" \
    "sh $13, 2(%0)\n\t" \
    "sh $14, 4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

void func_80055E84(SVECTOR *in, SVECTOR *out) {
    gte_ldv0(in);
    gte_rtv0tr();
    gte_stsv(out);
}
