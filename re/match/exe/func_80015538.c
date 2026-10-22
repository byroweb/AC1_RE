#include "ac1/ac1.h"

s32 *func_80015538(s32 *p, s32 v, s32 n)
{
    while (n--) {
        *p++ = v;
    }
    return p;
}
