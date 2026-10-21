#include "ac1/ac1.h"
/* ovl202 0x8006FB6C - bitset iterator: func_8006FB6C(bits, nwords) returns the first set
 * bit, func_8006FB6C(0) the next one, -1 at the end.  The word count is read through
 * &bits (the varargs home slot); the state lives in five file statics. */
extern u32 *D_800908D8;
extern u32 D_800908DC;
extern s32 D_800908E0;
extern s32 D_800908E4;
extern s32 D_800908E8;

s32 func_8006FB6C(u32 *bits, ...)
{
    if (bits == 0) {
        goto next_bit;
    }
    D_800908E0 = 0;
    D_800908D8 = bits;
    D_800908E4 = ((s32 *)&bits)[1];
    do {
        D_800908DC = *D_800908D8++;
        if (D_800908DC != 0) {
            D_800908E8 = 32;
            for (;;) {
                if (D_800908DC & 1) {
                    return D_800908E0;
                }
next_bit:
                D_800908DC >>= 1;
                D_800908E8--;
                D_800908E0++;
                if (D_800908E8 == 0) {
                    break;
                }
            }
        } else {
            D_800908E0 += 32;
        }
    } while (--D_800908E4 != 0);
    return -1;
}
