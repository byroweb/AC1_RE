#include "ac1/ac1.h"
/* ovl202 0x8006DC3C - enter world struct x in the x / y / z bucket rows its box covers */
void func_8006DB90(u32 (*map)[16], s32 y0, s32 y1, u32 x);
extern u32 D_801EA790[3][64][16];

void func_8006DC3C(CollBox *b, u32 x)
{
    func_8006DB90(D_801EA790[0], (b->min.vx >> 10) + 32, (b->max.vx >> 10) + 32, x);
    func_8006DB90(D_801EA790[1], (b->min.vy >> 10) + 32, (b->max.vy >> 10) + 32, x);
    func_8006DB90(D_801EA790[2], (b->min.vz >> 10) + 32, (b->max.vz >> 10) + 32, x);
}
