#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_dpcs() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A780010")

/* depth-cue the colour already in RGB toward the far colour by p */
void func_8006466C(s32 p)
{
    gte_lddp(p);
    gte_dpcs();
}
