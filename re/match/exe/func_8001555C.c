#include "ac1/ac1.h"

s16 *func_8001555C(s16 *p, s16 v, s32 n)
{
    while (n--) {
        *p++ = v;
    }
    return p;
}
