#include "ac1/ac1.h"

extern void func_8004E98C(void *, s32, s32, s32);
extern s32 func_8004EBF8(void *);
extern void func_80076E20(void *, s32, s32, void *);
extern u8 D_8007618C[];

s32 func_801C4FD8(u8 *o)
{
    switch (o[0x64]) {
    case 2:
    case 4:
    case 6:
        func_80076E20(o, 7, 1, D_8007618C);
        *(s16 *)(o + 0x72) = 0x100;
        return 1;
    case 7:
        if (func_8004EBF8(o + 0x64) != 0) {
            return 1;
        }
        func_80076E20(o, 7, 1, D_8007618C);
        return 1;
    case 15:
    case 17:
    case 19:
        func_80076E20(o, 20, 1, D_8007618C);
        *(s16 *)(o + 0x72) = 0x100;
        return 1;
    case 20:
        if (func_8004EBF8(o + 0x64) != 0) {
            return 1;
        }
        func_80076E20(o, 20, 1, D_8007618C);
        return 1;
    case 9:
        func_80076E20(o, 10, 1, D_8007618C);
        *(s16 *)(o + 0x72) = 0x100;
        return 1;
    case 10:
        if (func_8004EBF8(o + 0x64) != 0) {
            return 1;
        }
        func_80076E20(o, 10, 1, D_8007618C);
        return 1;
    case 12:
        func_80076E20(o, 13, 1, D_8007618C);
        *(s16 *)(o + 0x72) = 0x100;
        return 1;
    case 13:
        if (func_8004EBF8(o + 0x64) != 0) {
            return 1;
        }
        func_80076E20(o, 13, 1, D_8007618C);
        return 1;
    case 21:
        func_8004E98C(o + 0x64, 0, 1, 0x100);
        o[0x66] = 4;
        *(s16 *)(o + 0xCE) = 0;
        *(s16 *)(o + 0xCC) = 0;
        return 1;
    }
    return 1;
}
