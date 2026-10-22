#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 rand(void);
extern s32 func_80073714(AcEntity *, s32);
extern s16 func_80073904(void);
extern void func_80075A90(AcEntity *, s32, s16 *);
extern void func_801C5F54(AcEntity *, s32);

s32 func_801C5FB4(AcEntity *e, s32 msg, ...)
{
    s32 dist;
    u32 v;
    s16 sp[3];

    dist = func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 4:
        if (e->ai_mode != 0xFF) {
            e->ai_mode = 0;
        }
        e->unk3a[2] = 7;
        e->state = 7;
        e->unk42[0] = 0;
        *(u16 *)&e->unk42[2] = func_80073904();
        return 0;
    case 1:
    case 2:
        if (e->unk42[0] == 0) {
            goto start;
        }
        e->unk42[0] = 1;
        *(u16 *)&e->unk42[2] = func_80073904();
        v = rand() & 0xF;
        goto dispatch;
    default:
        switch (e->unk42[0]) {
        case 0:
            if (dist < 15000 || M2C_FIELD(e, u8 *, 0xFE) != 0) {
start:
                e->unk42[0] = 1;
                *(u16 *)&e->unk42[2] = func_80073904();
                func_801C5F54(e, 1);
            }
            return 0;
        case 1:
            if (dist > 18000 && M2C_FIELD(e, u8 *, 0xFE) == 0) {
                e->unk42[0] = 0;
                *(u16 *)&e->unk42[2] = func_80073904();
                sp[2] = 0;
                sp[0] = 0;
                sp[1] = 0x7FFF;
                func_80075A90(e, 2, sp);
            }
            v = 34;
            if (dist < 1601) {
                if (rand() & 1) {
                    v = (rand() & 3) + 8;
                } else {
                    v = rand() & 0x1F;
                }
            }
dispatch:
            switch (v) {
            case 0: case 1: case 2: case 3: case 4: case 5:
                func_801C5F54(e, 3);
                break;
            case 6: case 7:
                func_801C5F54(e, 2);
                break;
            case 8: case 9:
                func_801C5F54(e, 5);
                break;
            case 10: case 11:
                func_801C5F54(e, 6);
                break;
            case 32:
                func_801C5F54(e, 8);
                break;
            case 33:
                func_801C5F54(e, 9);
                break;
            case 34:
                func_801C5F54(e, 10);
                break;
            default:
                break;
            }
            break;
        }
        if (M2C_FIELD(e, u8 *, 0xFE) != 0) {
            M2C_FIELD(e, u8 *, 0xFE) = M2C_FIELD(e, u8 *, 0xFE) - 1;
        }
        break;
    case 5:
        break;
    }
    return 0;
}
