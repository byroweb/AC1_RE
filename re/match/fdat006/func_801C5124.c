#include "ac1/ac1.h"

extern s32 rand(void);
extern s32 func_80073714(AcEntity *, s32);
extern void func_80075A18(AcEntity *, s32, void *);
extern void func_80075A54(AcEntity *, s32, void *);
extern void func_801C50C0(AcEntity *, s32);
extern u8 D_80039C78[];

s32 func_801C5124(AcEntity *e, s32 msg, ...)
{
    s32 dist;
    u32 v;

    dist = func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 5:
        if (e->state == 1) {
            e->state = 2;
        } else if (e->state == 2) {
            e->state = 1;
        }
        return 0;
    case 1:
    case 2:
        if (rand() < 10814) {
            v = rand() & 0xF;
            goto dispatch;
        }
        return 0;
    }
    if (e->unk42[0] == 1) {
        if (dist > 1000) {
            func_801C50C0(e, 1);
        } else if (dist < 500) {
            func_801C50C0(e, 2);
        } else {
            if ((u32)(e->state - 3) < 2) {
                v = rand() & 7;
            } else {
                v = rand() & 0x3F;
            }
dispatch:
            switch (v) {
            case 0: case 1: case 2: case 3: case 4: case 5:
                func_801C50C0(e, 1);
                break;
            case 6: case 7:
                func_801C50C0(e, 2);
                break;
            case 8: case 9:
                func_80075A18(e, 1, D_80039C78);
                break;
            case 10: case 11:
                func_80075A54(e, 1, D_80039C78);
                break;
            case 12: case 13:
                func_80075A18(e, 2, D_80039C78);
                break;
            case 14: case 15:
                func_80075A54(e, 2, D_80039C78);
                break;
            case 16: case 17: case 18: case 19:
                func_801C50C0(e, 3);
                break;
            case 20: case 21: case 22: case 23:
                func_801C50C0(e, 4);
                break;
            }
        }
    }
    return 0;
}
