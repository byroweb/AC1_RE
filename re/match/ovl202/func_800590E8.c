#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_ldv1(r0) __asm__ volatile ("lwc2 $2, 0(%0)\n\tlwc2 $3, 4(%0)" : : "r"(r0))
#define gte_ldv2(r0) __asm__ volatile ("lwc2 $4, 0(%0)\n\tlwc2 $5, 4(%0)" : : "r"(r0))
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(r0))
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(r0) : "memory")
#define gte_stdp(r0) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(r0) : "$12", "memory")

#define gte_ldsv(r0) __asm__ volatile ("lhu $12, 0(%0)\n\tlhu $13, 2(%0)\n\tlhu $14, 4(%0)\n\tmtc2 $12, $9\n\tmtc2 $13, $10\n\tmtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_cmd_4A680029() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A680029")

void func_800590E8(SVECTOR *ir, void *rgb, s32 p)
{
    gte_ldsv(ir);
    gte_ldrgb(rgb);
    gte_lddp(p);
    gte_cmd_4A680029();
}
