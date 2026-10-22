#include "ac1/ac1.h"

void func_80014F18(u16 *a, u16 *b, u16 *out, s16 t)
{
    s32 i;
    u16 va;
    u16 vb;
    u16 *p;

    p = a;
    i = 8;
    do {
        vb = *b;
        va = *p;
        p++;
        b++;
        i--;
        *out = va + ((((s16)vb - (s16)va) * t) >> 12);
        out++;
    } while (i != -1);
}
