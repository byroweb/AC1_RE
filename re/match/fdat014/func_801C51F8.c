#include "ac1/ac1.h"

extern s32 func_8004EAD0(void *);
extern void func_800766E0(void *, s32, s32, s32, s32, void *, void *);
extern s32 func_801C50F4(void *);
extern void func_801C4EE0();

void func_801C51F8(u8 *o, s32 arg1, s32 arg2)
{
    if (o[0x64] == 2 || o[0x64] == 5) {
        func_800766E0(o, arg1, 1, 1, 0, 0, func_801C4EE0);
        *(s16 *)(o + 0x72) = 0x100;
        return;
    }
    if (o[0x64] == arg2) {
        if (func_8004EAD0(o + 0x64) != 0) {
            func_800766E0(o, arg1, 1, 0, 0, 0, func_801C4EE0);
            return;
        }
        func_800766E0(o, arg2, 1, 0, 0, 0, func_801C4EE0);
        return;
    }
    if (func_801C50F4(o) != 0) {
        func_800766E0(o, arg2, 1, 0, 0, 0, func_801C4EE0);
        *(s16 *)(o + 0x72) = 0x100;
    }
}
