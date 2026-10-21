#include "ac1/ac1.h"
void func_800131D0(void *p, s32 a, s32 b);

void func_8004DFF8(u8 *o, s32 v, s16 *r)
{
    o[3] = o[2] = v;
    func_800131D0(o + 0x20, v, r[0]);
    func_800131D0(o + 0x28, v, r[1]);
    func_800131D0(o + 0x30, v, r[2]);
}
