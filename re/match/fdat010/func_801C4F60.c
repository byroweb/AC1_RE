#include "ac1/ac1.h"

extern u32 func_80014D78(s32);
extern s32 func_80073714(AcEntity *, s32);
extern void func_801C4EFC(AcEntity *, s32);

s32 func_801C4F60(AcEntity *e, s32 msg, ...)
{
    s32 dist;
    u32 r;

    dist = func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 5:
        switch (e->state) {
        case 1:
            e->state = 2;
            break;
        case 2:
            e->state = 1;
            break;
        }
        return 0;
    case 1:
    case 2:
        return 0;
    }
    if (e->unk42[0] == 1) {
        if (dist > 2500) {
            func_801C4EFC(e, 1);
        } else if (dist < 1000) {
            func_801C4EFC(e, 2);
        } else {
            r = func_80014D78(30);
            switch (r) {
            case 0:
            case 1:
                func_801C4EFC(e, 1);
                break;
            case 2:
            case 3:
                func_801C4EFC(e, 2);
                break;
            case 4:
            case 5:
                func_801C4EFC(e, 0);
                break;
            }
        }
    }
    return 0;
}
