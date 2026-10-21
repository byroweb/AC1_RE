#include "ac1/ac1.h"
/* ovl202 0x80071DC4 - if func_80071ADC(arg0, 1) passes, the record func_80071D4C(arg1) picks */

s32 func_80071ADC(s32 a, s32 b);
s32 func_80071D4C(s32 mask);

s32 func_80071DC4(s32 arg0, s32 arg1)
{
    s32 p;

    if (func_80071ADC(arg0, 1) != 0) {
        p = func_80071D4C(arg1);
        if (p != 0) {
            return p;
        } else {
            return 0;
        }
    }
    return 0;
}
