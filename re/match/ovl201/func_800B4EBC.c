#include "ac1/ac1.h"

extern s32 D_800BE218[16];
extern s32 D_800BE258[16];
extern s32 D_800BE29C[32];

/* copy the three default tables (16 + 16 + 32 words) into dst */
s32 *func_800B4EBC(s32 *dst)
{
    s32 *d;
    s32 *s;
    s32 i;

    d = dst;
    s = D_800BE218;
    for (i = 15; i != -1; i--) {
        *d++ = *s++;
    }
    d = dst + 16;
    s = D_800BE258;
    for (i = 15; i != -1; i--) {
        *d++ = *s++;
    }
    d = dst + 32;
    s = D_800BE29C;
    for (i = 31; i != -1; i--) {
        *d++ = *s++;
    }
    return dst;
}
