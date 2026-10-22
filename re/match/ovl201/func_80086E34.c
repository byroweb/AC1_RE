#include "ac1/ac1.h"

extern u8 D_801F3814[];
extern volatile s8 D_801F37C8;
extern void func_800154D8(void *dst, void *src, s32 n);

void func_80086E34(void *buf)
{
    func_800154D8(D_801F3814, buf, 0x620);
    D_801F37C8 = 0;
    free2(buf);
}
