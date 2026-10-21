#include "ac1/ac1.h"

extern s32 func_8004EAD0(void *);
extern void func_800766E0(void *, s32, s32, s32, s32, void *, void *);
extern s32 func_801C4FD8(void *);
extern void func_801C4DD8();
extern u8 D_801C6CF0[][8];

void func_801C5128(u8 *o, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    switch (o[0x64]) {
    case 2:
    case 4:
    case 6:
    case 9:
    case 12:
    case 15:
    case 17:
    case 19:
        func_800766E0(o, arg1, 1, 1, 0, 0, func_801C4DD8);
        *(s16 *)(o + 0x72) = 0x100;
        return;
    }
    if (o[0x64] == arg2) {
        if (func_8004EAD0(o + 0x64) != 0) {
            func_800766E0(o, arg1, 1, 0, 2, D_801C6CF0[o[0xFC]], func_801C4DD8);
            return;
        }
        func_800766E0(o, arg2, 1, 0, 0, 0, func_801C4DD8);
        return;
    }
    if (o[0x64] == arg3 || o[0x64] == arg4) {
        if (func_8004EAD0(o + 0x64) != 0) {
            func_800766E0(o, arg1, 1, 0, 2, D_801C6CF0[o[0xFC]], func_801C4DD8);
            return;
        }
        func_800766E0(o, arg2, 1, 0, 0, 0, func_801C4DD8);
        return;
    }
    if (func_801C4FD8(o) != 0) {
        func_800766E0(o, arg2, 1, 0, 0, 0, func_801C4DD8);
        *(s16 *)(o + 0x72) = 0x100;
    }
}
