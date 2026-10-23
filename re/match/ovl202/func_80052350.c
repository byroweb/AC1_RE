#include "ac1/ac1.h"

/* Parse a decimal number at *pp, advance *pp past it.
 * Retail-identical when cc1 runs without ASLR (gdb): the Windows cc1 crashes in
 * reorg.c fill_slots_from_thread (implicit int negate_rtx truncates the rtx pointer). */
s32 func_80052350(u8 **pp)
{
    u8 *p = *pp;
    s32 c = *p;
    s32 n = 0;

    while ((u32)(c - '0') < 10) {
        n = c + n * 10 - '0';
        c = *++p;
    }
    *pp = p;
    return n;
}
