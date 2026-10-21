#include "ac1/ac1.h"
/* ovl202 0x8006DDC4 - candidate world structs at a point: AND of the x, y and z
 * bucket bitsets (64 buckets of 1024 units per axis, D_801E8B70 words each) */
extern u32 D_801EAF90[3][64][16];
extern s32 D_801E8B70;

void func_8006DDC4(SVECTOR *v, u32 *out)
{
    u32 *x;
    u32 *y;
    u32 *z;
    s32 n;

    x = D_801EAF90[0][v->vx >> 10];
    y = D_801EAF90[1][v->vy >> 10];
    z = D_801EAF90[2][v->vz >> 10];
    n = D_801E8B70;
    do {
        *out++ = *x++ & *y++ & *z++;
    } while (--n != 0);
}
