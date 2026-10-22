#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldsxy3(r0, r1, r2) __asm__ volatile ("mtc2 %0, $12\n\tmtc2 %2, $14\n\tmtc2 %1, $13" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_nclip() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B400006")
#define gte_stopz(r0) __asm__ volatile ("swc2 $24, 0(%0)" : : "r"(r0) : "memory")

/* outer product (winding) of three screen points */
void func_80058028(s32 *opz, s32 xy0, s32 xy1, s32 xy2)
{
    gte_ldsxy3(xy0, xy1, xy2);
    gte_nclip();
    gte_stopz(opz);
}
