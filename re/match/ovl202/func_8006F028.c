#include "ac1/ac1.h"
/* ovl202 0x8006F028 - PsyQ gte_NormalClip: NCLIP of three screen XYs, MAC0 to *out */

#define gte_ldsxy3(r0, r1, r2) __asm__ volatile ( \
    "mtc2 %0, $12\n\t" \
    "mtc2 %2, $14\n\t" \
    "mtc2 %1, $13" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_nclip() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B400006")
#define gte_stopz(r0) __asm__ volatile ( \
    "swc2 $24, 0(%0)" : : "r"(r0) : "memory")

void func_8006F028(s32 sxy0, s32 sxy1, s32 sxy2, s32 *opz)
{
    gte_ldsxy3(sxy0, sxy1, sxy2);
    gte_nclip();
    gte_stopz(opz);
}
