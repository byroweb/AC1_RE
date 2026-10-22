#include "ac1/ac1.h"
extern volatile s8 D_8019C090;
extern u8 D_8019C360[];
extern void func_800154D8(void *dst, void *src, s32 n);

void func_8008C348(UiObj *o)
{
    func_800154D8(D_8019C360, o, 0xC31);
    D_8019C090 = 0;
    free2(o);
}
