#include "ac1/ac1.h"

u8 *func_80015440(u8 *p, s32 n, s32 size, s32 (*cmp)(void *a, void *b, s32 arg), s32 arg)
{
    u8 *best;

    best = p;
    n--;
    while (n != 0) {
        p += size;
        if (cmp(best, p, arg) > 0) {
            best = p;
        }
        n--;
    }
    return best;
}
