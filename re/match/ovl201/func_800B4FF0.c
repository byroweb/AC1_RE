#include "ac1/ac1.h"

extern void func_800B5248(u32 *ctl, u32 mode);

void func_800B4FF0(u32 *ctl, s32 flags)
{
    if (flags & 1) {
        *ctl &= ~0x08000000;
    } else {
        *ctl |= 0x08000000;
    }
    if (flags & 2) {
        *ctl |= 0x02000000;
    } else {
        *ctl &= ~0x02000000;
    }
    func_800B5248(ctl, *(u16 *)ctl);
}
