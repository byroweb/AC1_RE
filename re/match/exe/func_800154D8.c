#include "ac1/ac1.h"

s32 *func_800154D8(s32 *d, s32 *s, s32 n)
{
    while (n--) {
        *d++ = *s++;
    }
    return s;
}
