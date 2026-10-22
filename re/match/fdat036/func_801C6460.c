#include "ac1/ac1.h"

extern s32 func_80073714();
extern void func_801C6320();
extern void func_801C6390();
extern void func_801C63F8();
extern s32 rand();

s32 func_801C6460(s32 a0, s32 a1, ...)
{
    s32 t;

    t = func_80073714(a0, 0x7FFFFFFF);
    if (a1 != 5 && a1 != 0x8100) {
        if (t >= 0x1B59) {
            func_801C6320(a0, 2);
        } else {
            switch (rand() & 0x3F) {
            case 0:
            case 1:
            case 2:
                func_801C6320(a0, 2);
                break;
            case 3:
                func_801C6320(a0, 3);
                break;
            case 4:
            case 5:
                func_801C6390(a0, 2);
                break;
            case 6:
            case 7:
                func_801C63F8(a0, 2);
                break;
            case 8:
                func_801C6390(a0, 3);
                break;
            case 11:
                func_801C63F8(a0, 3);
                break;
            case 12:
                func_801C6320(a0, 4);
                break;
            case 13:
                func_801C6320(a0, 5);
                break;
            case 14:
                func_801C6320(a0, 1);
                break;
            }
        }
    }
    return 0;
}
