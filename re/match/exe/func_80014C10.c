#include "ac1/ac1.h"

s32 func_80014C10(s32 a, s32 b, s32 c, s32 d)
{
    if (c < a) {
        if (c < a - b) {
            return 0;
        }
    } else {
        if (a < c - d) {
            return 0;
        }
    }
    return 1;
}
