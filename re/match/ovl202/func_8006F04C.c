#include "ac1/ac1.h"
/* ovl202 0x8006F04C - first row of a rotation matrix dotted with an SVECTOR
 * (GTE MVMVA rt*ir, sf=0), MAC1 to *out */

#define gte_ldr0(r0) __asm__ volatile ( \
    "lw $12, 0(%0)\n\t" \
    "lw $13, 4(%0)\n\t" \
    "ctc2 $12, $0\n\t" \
    "ctc2 $13, $1" : : "r"(r0) : "$12", "$13")
#define gte_ldsv(r0) __asm__ volatile ( \
    "lhu $12, 0(%0)\n\t" \
    "lhu $13, 2(%0)\n\t" \
    "lhu $14, 4(%0)\n\t" \
    "mtc2 $12, $9\n\t" \
    "mtc2 $13, $10\n\t" \
    "mtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_rtir_sf0() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A41E012")
#define gte_stlvnl0(r0) __asm__ volatile ( \
    "swc2 $25, 0(%0)" : : "r"(r0) : "memory")

void func_8006F04C(MATRIX *m, SVECTOR *v, s32 *out)
{
    gte_ldr0(m);
    gte_ldsv(v);
    gte_rtir_sf0();
    gte_stlvnl0(out);
}
