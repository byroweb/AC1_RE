#include "ac1/ac1.h"

extern void func_80078598(void *);
extern void func_8007732C(void *, s32, s32, s32, void *, void *);
extern void func_801C6528();

void func_801C6688(u8 *arg0, s8 arg1)
{
    arg0[0x3C] = arg1;
    func_80078598(arg0);
    func_8007732C(arg0, 0, 0, 0, arg0 + 0xCC, func_801C6528);
}
