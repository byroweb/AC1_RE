#include "ac1/ac1.h"

u16 *func_80015508(u16 *d, u16 *s, s32 n)
{
    while (n--) {
        *d++ = *s++;
    }
    return s;
}
