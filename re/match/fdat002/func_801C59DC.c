#include "ac1/ac1.h"

extern u32 func_80014D78(s32);
extern s32 func_80073714(void *, s32);
extern void func_801C5978(void *, s32);
extern s32 rand(void);

s32 func_801C59DC(u8 *o, s32 msg, ...)
{
    s32 d;
    u8 v;

    d = func_80073714(o, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 5:
        switch (o[0x41]) {
        case 1:
            v = 6;
            goto store;
        case 2:
            v = 8;
            goto store;
        case 3:
            v = 7;
            goto store;
        case 4:
            v = 5;
            goto store;
        case 5:
            v = 4;
            goto store;
        case 6:
            v = 1;
            goto store;
        case 7:
            v = 3;
            goto store;
        case 8:
            v = 2;
        store:
            o[0x41] = v;
            break;
        }
        break;
    case 1:
    case 2:
        if (rand() >= 10814) {
            return 0;
        }
        if (rand() < 16384) {
            func_801C5978(o, 1);
        } else {
            func_801C5978(o, 6);
        }
        break;
    default:
        if (o[0x42] != 1) {
            return 0;
        }
        if (d > 2500) {
            func_801C5978(o, 1);
        } else if (d < 1000) {
            func_801C5978(o, 6);
        } else {
            switch (func_80014D78(80)) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                func_801C5978(o, 1);
                break;
            case 6:
            case 7:
                func_801C5978(o, 2);
                break;
            case 8:
            case 9:
                func_801C5978(o, 3);
                break;
            case 10:
            case 11:
                func_801C5978(o, 4);
                break;
            case 12:
            case 13:
                func_801C5978(o, 5);
                break;
            case 14:
            case 15:
                func_801C5978(o, 6);
                break;
            case 16:
            case 17:
                func_801C5978(o, 7);
                break;
            case 18:
            case 19:
                func_801C5978(o, 8);
                break;
            case 28:
            case 29:
            case 30:
            case 31:
                func_801C5978(o, 9);
                break;
            case 32:
            case 33:
            case 34:
            case 35:
                func_801C5978(o, 10);
                break;
            }
        }
        break;
    }
    return 0;
}
