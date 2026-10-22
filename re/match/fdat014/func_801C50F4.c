#include "ac1/ac1.h"

extern s32 func_8004EBF8(void *);
extern void func_800765E8(void *, s32, s32, void *);
extern void func_80076E20(void *, s32, s32, void *);
extern u8 D_8007618C[];
extern void func_801C4E00();

s32 func_801C50F4(u8 *o)
{
    switch (o[0x64]) {
    case 2:
        func_80076E20(o, 3, 1, D_8007618C);
        *(s16 *)(o + 0x72) = 0x100;
        return 1;
    case 3:
        if (func_8004EBF8(o + 0x64) != 0) {
            return 1;
        }
        func_80076E20(o, 3, 1, D_8007618C);
        return 1;
    case 5:
        func_80076E20(o, 6, 1, D_8007618C);
        *(s16 *)(o + 0x72) = 0x100;
        return 1;
    case 6:
        if (func_8004EBF8(o + 0x64) != 0) {
            return 1;
        }
        func_80076E20(o, 6, 1, D_8007618C);
        return 1;
    case 8:
        if (func_8004EBF8(o + 0x64) != 0) {
            return 1;
        }
        func_800765E8(o, 8, 1, func_801C4E00);
        return 0;
    }
    return 1;
}
