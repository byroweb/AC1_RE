#include "ac1/ac1.h"

extern s32 rand(void);
extern s32 func_80073714(AcEntity *, s32);
extern void func_801C66B4(AcEntity *, s32);
extern void func_801C6724(AcEntity *, s32);
extern void func_801C678C(AcEntity *, s32);

s32 func_801C67F4(AcEntity *e, s32 msg, ...)
{
    s32 dist;
    s32 v;

    dist = func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 4:
        if (e->ai_mode != 0xFF) {
            e->ai_mode = 0;
        }
        e->faction &= ~0x10;
        goto near;
    case 1:
    case 2:
        v = rand() & 0xF;
        goto dispatch;
    default:
near:
        if (dist > 7000) {
            func_801C66B4(e, 2);
            return 0;
        }
        v = rand() & 0x3F;
dispatch:
        switch (v) {
        case 0:
        case 1:
        case 2:
            func_801C66B4(e, 2);
            break;
        case 3:
            func_801C66B4(e, 3);
            break;
        case 4:
        case 5:
            func_801C6724(e, 2);
            break;
        case 6:
        case 7:
            func_801C678C(e, 2);
            break;
        case 8:
            func_801C6724(e, 3);
            break;
        case 11:
            func_801C678C(e, 3);
            break;
        case 12:
            func_801C66B4(e, 4);
            break;
        case 13:
            func_801C66B4(e, 5);
            break;
        case 14:
            func_801C66B4(e, 1);
            break;
        }
        break;
    case 5:
        break;
    }
    return 0;
}
