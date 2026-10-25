/* include/ac1/gte.h - PsyQ inline_c.h GTE macros, as the retail code uses them.
 *
 * Not part of the ac1.h umbrella (m2c's parser does not take inline asm):
 * include it next to "ac1/ac1.h" in a source that needs the GTE.  Each macro
 * is the PsyQ shape the retail bytes show; $12..$14 are t4..t6.  The forms are
 * the ones matched C uses (many older matches still define them locally; a local
 * #define after this include wins).  Names and cop2 words follow PsyQ, so a
 * local `gte_rtv0` that emits 0x4A480012 is this file's gte_rtv0tr.
 *
 * Reading retail asm: `ctc2 t4..t6, $0..$4` after `lw` from one base is
 * gte_SetRotMatrix, `$5..$7` gte_SetTransMatrix, `$8..$12` gte_SetLightMatrix;
 * `nop; nop; cop2 W` is the command whose word is W (list below).
 */
#ifndef AC1_GTE_H
#define AC1_GTE_H

/* ---- loads ---- */

/* load SVECTOR *r0 into V0 */
#define gte_ldv0(r0) __asm__ volatile ( \
    "lwc2 $0, 0(%0)\n\t" \
    "lwc2 $1, 4(%0)" : : "r"(r0))

/* load SVECTOR *r0 into V1 */
#define gte_ldv1(r0) __asm__ volatile ( \
    "lwc2 $2, 0(%0)\n\t" \
    "lwc2 $3, 4(%0)" : : "r"(r0))

/* load SVECTOR *r0 into V2 */
#define gte_ldv2(r0) __asm__ volatile ( \
    "lwc2 $4, 0(%0)\n\t" \
    "lwc2 $5, 4(%0)" : : "r"(r0))

/* load SVECTORs *r0, *r1, *r2 into V0, V1, V2 */
#define gte_ldv3(r0, r1, r2) __asm__ volatile ( \
    "lwc2 $0, 0(%0)\n\t" \
    "lwc2 $1, 4(%0)\n\t" \
    "lwc2 $2, 0(%1)\n\t" \
    "lwc2 $3, 4(%1)\n\t" \
    "lwc2 $4, 0(%2)\n\t" \
    "lwc2 $5, 4(%2)" : : "r"(r0), "r"(r1), "r"(r2))

/* load VECTOR *r0 (32-bit x, y, z) into V0 */
#define gte_ldlv0(r0) __asm__ volatile ( \
    "lhu $13, 4(%0)\n\t" \
    "lhu $12, 0(%0)\n\t" \
    "sll $13, $13, 16\n\t" \
    "or $12, $12, $13\n\t" \
    "mtc2 $12, $0\n\t" \
    "lwc2 $1, 8(%0)" : : "r"(r0) : "$12", "$13")

/* load VECTOR *r0 into IR1..IR3 */
#define gte_ldlvl(r0) __asm__ volatile ( \
    "lwc2 $9, 0(%0)\n\t" \
    "lwc2 $10, 4(%0)\n\t" \
    "lwc2 $11, 8(%0)" : : "r"(r0))

/* load SVECTOR *r0 into IR1..IR3 */
#define gte_ldsv(r0) __asm__ volatile ( \
    "lhu $12, 0(%0)\n\t" \
    "lhu $13, 2(%0)\n\t" \
    "lhu $14, 4(%0)\n\t" \
    "mtc2 $12, $9\n\t" \
    "mtc2 $13, $10\n\t" \
    "mtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")

/* load a MATRIX column (*r0, stride 6) into IR1..IR3 */
#define gte_ldclmv(r0) __asm__ volatile ( \
    "lhu $12, 0(%0)\n\t" \
    "lhu $13, 6(%0)\n\t" \
    "lhu $14, 12(%0)\n\t" \
    "mtc2 $12, $9\n\t" \
    "mtc2 $13, $10\n\t" \
    "mtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")

/* load CVECTOR *r0 (r, g, b bytes) into IR1..IR3 */
#define gte_ldcv(r0) __asm__ volatile ( \
    "lbu $12, 0(%0)\n\t" \
    "lbu $13, 1(%0)\n\t" \
    "lbu $14, 2(%0)\n\t" \
    "mtc2 $12, $9\n\t" \
    "mtc2 $13, $10\n\t" \
    "mtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")

/* load CVECTOR *r0 into RGBC */
#define gte_ldrgb(r0) __asm__ volatile ( \
    "lwc2 $6, 0(%0)" : : "r"(r0))

/* load the depth-cue interpolation value r0 into IR0 */
#define gte_lddp(r0) __asm__ volatile ( \
    "mtc2 %0, $8" : : "r"(r0))

/* load *r0 into IR0 */
#define gte_ldir0(r0) __asm__ volatile ( \
    "lw $9, %0\n\t" \
    "mtc2 $9, $8" : : "m"(*(r0)) : "$9")

/* load screen XYs r0, r1, r2 into SXY0..SXY2 */
#define gte_ldsxy3(r0, r1, r2) __asm__ volatile ( \
    "mtc2 %0, $12\n\t" \
    "mtc2 %2, $14\n\t" \
    "mtc2 %1, $13" : : "r"(r0), "r"(r1), "r"(r2))

/* load screen Zs r0, r1, r2 into SZ1..SZ3 */
#define gte_ldsz3(r0, r1, r2) __asm__ volatile ( \
    "mtc2 %0, $17\n\t" \
    "mtc2 %1, $18\n\t" \
    "mtc2 %2, $19" : : "r"(r0), "r"(r1), "r"(r2))

/* load r0 into LZCS (count leading zeros/ones) */
#define gte_ldlzc(r0) __asm__ volatile ( \
    "mtc2 %0, $30\n\t" \
    "nop" : : "r"(r0))

/* ---- matrices (ctc2 from $12..$14 = t4..t6) ---- */

/* MATRIX *r0's rotation into RT */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12, 0(%0)\n\t" \
    "lw $13, 4(%0)\n\t" \
    "ctc2 $12, $0\n\t" \
    "ctc2 $13, $1\n\t" \
    "lw $12, 8(%0)\n\t" \
    "lw $13, 12(%0)\n\t" \
    "lw $14, 16(%0)\n\t" \
    "ctc2 $12, $2\n\t" \
    "ctc2 $13, $3\n\t" \
    "ctc2 $14, $4" : : "r"(r0) : "$12", "$13", "$14")

/* MATRIX *r0's translation into TR */
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12, 20(%0)\n\t" \
    "lw $13, 24(%0)\n\t" \
    "ctc2 $12, $5\n\t" \
    "lw $14, 28(%0)\n\t" \
    "ctc2 $13, $6\n\t" \
    "ctc2 $14, $7" : : "r"(r0) : "$12", "$13", "$14")

/* MATRIX *r0's rotation into the light matrix L */
#define gte_SetLightMatrix(r0) __asm__ volatile ( \
    "lw $12, 0(%0)\n\t" \
    "lw $13, 4(%0)\n\t" \
    "ctc2 $12, $8\n\t" \
    "ctc2 $13, $9\n\t" \
    "lw $12, 8(%0)\n\t" \
    "lw $13, 12(%0)\n\t" \
    "lw $14, 16(%0)\n\t" \
    "ctc2 $12, $10\n\t" \
    "ctc2 $13, $11\n\t" \
    "ctc2 $14, $12" : : "r"(r0) : "$12", "$13", "$14")

/* ---- commands: two nops, then the cop2 word (PsyQ inline_c.h) ---- */

/* perspective transform of V0 (RTPS) */
#define gte_rtps() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A180001")

/* perspective transform of V0..V2 (RTPT) */
#define gte_rtpt() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A280030")

/* RT * V0 + TR (MVMVA) */
#define gte_rt() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A480012")

/* RT * V0 (MVMVA, no translation) */
#define gte_rtv0() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A486012")

/* RT * V0 + TR (MVMVA) */
#define gte_rtv0tr() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A480012")

/* RT * IR (MVMVA, sf=1) */
#define gte_rtir() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A49E012")

/* RT * IR (MVMVA, sf=0) */
#define gte_rtir_sf0() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A41E012")

/* normal clip: SXY winding into MAC0 (NCLIP) */
#define gte_nclip() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B400006")

/* average of SZ1..SZ3 into OTZ (AVSZ3) */
#define gte_avsz3() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B58002D")

/* normal colour, single (NCS) */
#define gte_ncs() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4AC8041E")

/* normal colour colour, single (NCCS) */
#define gte_nccs() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B08041B")

/* normal colour colour, triple (NCCT) */
#define gte_ncct() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B18043F")

/* normal colour depth cue, single (NCDS) */
#define gte_ncds() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4AE80413")

/* normal colour depth cue, triple (NCDT) */
#define gte_ncdt() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4AF80416")

/* depth cue, single (DPCS) */
#define gte_dpcs() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4A780010")

/* IR * IR0 >> 12 (GPF, sf=1) */
#define gte_gpf12() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B98003D")

/* MAC + IR * IR0 >> 12 (GPL, sf=1) */
#define gte_gpl12() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4BA8003E")

/* IR squared (SQR, sf=0) */
#define gte_sqr0() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4AA00428")

/* IR squared >> 12 (SQR, sf=1) */
#define gte_sqr12() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4AA80428")

/* ---- stores ---- */

/* store IR1..IR3 to SVECTOR *r0 */
#define gte_stsv(r0) __asm__ volatile ( \
    "mfc2 $12, $9\n\t" \
    "mfc2 $13, $10\n\t" \
    "mfc2 $14, $11\n\t" \
    "sh $12, 0(%0)\n\t" \
    "sh $13, 2(%0)\n\t" \
    "sh $14, 4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

/* store IR1..IR3 to a MATRIX column *r0 (stride 6) */
#define gte_stclmv(r0) __asm__ volatile ( \
    "mfc2 $12, $9\n\t" \
    "mfc2 $13, $10\n\t" \
    "mfc2 $14, $11\n\t" \
    "sh $12, 0(%0)\n\t" \
    "sh $13, 6(%0)\n\t" \
    "sh $14, 12(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

/* store IR1..IR3 to CVECTOR *r0 (bytes) */
#define gte_stcv(r0) __asm__ volatile ( \
    "mfc2 $12, $9\n\t" \
    "mfc2 $13, $10\n\t" \
    "mfc2 $14, $11\n\t" \
    "sb $12, 0(%0)\n\t" \
    "sb $13, 1(%0)\n\t" \
    "sb $14, 2(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

/* store MAC1..MAC3 to VECTOR *r0 */
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25, 0(%0)\n\t" \
    "swc2 $26, 4(%0)\n\t" \
    "swc2 $27, 8(%0)" : : "r"(r0) : "memory")

/* store MAC1 to *r0 */
#define gte_stlvnl0(r0) __asm__ volatile ( \
    "swc2 $25, 0(%0)" : : "r"(r0) : "memory")

/* store SXY2 to *r0 */
#define gte_stsxy(r0) __asm__ volatile ( \
    "swc2 $14, 0(%0)" : : "r"(r0) : "memory")

/* store SXY0..SXY2 to *r0, *r1, *r2 */
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ( \
    "swc2 $12, 0(%0)\n\t" \
    "swc2 $13, 0(%1)\n\t" \
    "swc2 $14, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")

/* store SZ3 >> 2 (the ordering-table z) to *r0 */
#define gte_stszotz(r0) __asm__ volatile ( \
    "mfc2 $12, $19\n\t" \
    "nop\n\t" \
    "sra $12, $12, 2\n\t" \
    "sw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

/* store OTZ to *r0 */
#define gte_stotz(r0) __asm__ volatile ( \
    "swc2 $7, 0(%0)" : : "r"(r0) : "memory")

/* store MAC0 (the NCLIP result) to *r0 */
#define gte_stopz(r0) __asm__ volatile ( \
    "swc2 $24, 0(%0)" : : "r"(r0) : "memory")

/* store IR0 to *r0 */
#define gte_stdp(r0) __asm__ volatile ( \
    "swc2 $8, 0(%0)" : : "r"(r0) : "memory")

/* store RGB2 to *r0 */
#define gte_strgb(r0) __asm__ volatile ( \
    "swc2 $22, 0(%0)" : : "r"(r0) : "memory")

/* store RGB0..RGB2 to *r0, *r1, *r2 */
#define gte_strgb3(r0, r1, r2) __asm__ volatile ( \
    "swc2 $20, 0(%0)\n\t" \
    "swc2 $21, 0(%1)\n\t" \
    "swc2 $22, 0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")

/* store LZCR to *r0 */
#define gte_stlzc(r0) __asm__ volatile ( \
    "swc2 $31, 0(%0)" : : "r"(r0) : "memory")
#endif
