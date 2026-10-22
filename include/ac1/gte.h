/* include/ac1/gte.h - PsyQ inline_c.h GTE macros, as the retail code uses them.
 *
 * Not part of the ac1.h umbrella (m2c's parser does not take inline asm):
 * include it next to "ac1/ac1.h" in a source that needs the GTE.  Each macro
 * is the PsyQ shape the retail bytes show; $12..$14 are t4..t6.
 */
#ifndef AC1_GTE_H
#define AC1_GTE_H

/* load SVECTOR *r0 into V0 */
#define gte_ldv0(r0) __asm__ volatile ( \
    "lwc2 $0, 0(%0)\n\t" \
    "lwc2 $1, 4(%0)" : : "r"(r0))

/* perspective transform of V0 (RTPS) */
#define gte_rtps() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A180001")

/* store SXY2 to *r0 */
#define gte_stsxy(r0) __asm__ volatile ( \
    "swc2 $14, 0(%0)" : : "r"(r0) : "memory")

/* store SZ3 >> 2 (the ordering-table z) to *r0 */
#define gte_stszotz(r0) __asm__ volatile ( \
    "mfc2 $12, $19\n\t" \
    "nop\n\t" \
    "sra $12, $12, 2\n\t" \
    "sw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

/* load the depth-cue interpolation value r0 into IR0 */
#define gte_lddp(r0) __asm__ volatile ( \
    "mtc2 %0, $8" : : "r"(r0))

/* load SVECTOR *r0 into IR1..IR3 */
#define gte_ldsv(r0) __asm__ volatile ( \
    "lhu $12, 0(%0)\n\t" \
    "lhu $13, 2(%0)\n\t" \
    "lhu $14, 4(%0)\n\t" \
    "mtc2 $12, $9\n\t" \
    "mtc2 $13, $10\n\t" \
    "mtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")

/* general purpose interpolation IR * IR0 >> 12 (GPF, sf=1) */
#define gte_gpf12() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B98003D")

/* store IR1..IR3 to SVECTOR *r0 */
#define gte_stsv(r0) __asm__ volatile ( \
    "mfc2 $12, $9\n\t" \
    "mfc2 $13, $10\n\t" \
    "mfc2 $14, $11\n\t" \
    "sh $12, 0(%0)\n\t" \
    "sh $13, 2(%0)\n\t" \
    "sh $14, 4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

#endif
