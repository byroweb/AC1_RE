#include "ac1/ac1.h"
/* ovl202 0x80067EA4 - load RGB from *rgb and run NCCS (normal colour, single). */

/* PsyQ inline_c.h-style GTE macros, local to this file. */
#define gte_ldrgb(r0) __asm__ volatile ("lwc2 $6, 0( %0 )" : : "r"(r0))
#define gte_nccs() __asm__ volatile ("nop;nop;.word 0x4B08041B")

void func_80067EA4(u8 *rgb)
{
    gte_ldrgb(rgb);
    gte_nccs();
}
