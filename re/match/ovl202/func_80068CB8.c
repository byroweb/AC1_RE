#include "ac1/ac1.h"
/* ovl202 0x80068CB8 - normal clip (NCLIP) of three packed screen xy: stores OPZ to *out and returns it. */

/* PsyQ inline_c.h-style GTE macros, local to this file. */
#define gte_ldsxy3(r0, r1, r2) __asm__ volatile ("mtc2 %0, $12;mtc2 %2, $14;mtc2 %1, $13" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_nclip() __asm__ volatile ("nop;nop;.word 0x4B400006")
#define gte_stopz(r0) __asm__ volatile ("swc2 $24, 0( %0 )" : : "r"(r0) : "memory")

s32 func_80068CB8(s32 *out, s32 xy0, s32 xy1, s32 xy2)
{
    gte_ldsxy3(xy0, xy1, xy2);
    gte_nclip();
    gte_stopz(out);
    return *out;
}
