#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 rand(void);
extern s32 func_80073714(AcEntity *, s32);
extern s16 func_80073904(void);
extern void func_80078598(AcEntity *);
extern s32 func_800705D8(s32 *, void *, s32);
extern void func_801C5D24(AcEntity *, s32, s32);
extern void func_801C5DA0(AcEntity *, s32, s32);
extern void func_801C5E14(AcEntity *, s32, s32);

s32 func_801C5E88(AcEntity *e, s32 msg, ...)
{
    s32 p[6];
    u8 buf[8];
    s32 w;
    u32 v;

    func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 4:
        if (e->ai_mode != 0xFF) {
            func_80078598(e);
            if (M2C_FIELD(e, s16 *, 0x2E) != 0x7FFF) {
                if (M2C_FIELD(e, s16 *, 0x2C) != 0x7FFF) {
                    e->pos.y = M2C_FIELD(e, s16 *, 0x2C);
                }
            }
            e->ai_mode = 0;
        }
        goto check;
    case 1:
    case 2:
        e->unk42[0] = 0;
        *(u16 *)&e->unk42[2] = func_80073904();
        v = rand() & 0xF;
        goto dispatch;
    default:
        p[0] = e->pos.x;
        p[3] = e->pos.x + ((s16)M2C_FIELD(e, u16 *, 0xCC) >> 4);
        p[1] = e->pos.y;
        p[4] = e->pos.y + ((s16)M2C_FIELD(e, u16 *, 0xCE) >> 4);
        p[2] = e->pos.z;
        p[5] = e->pos.z + ((s16)M2C_FIELD(e, u16 *, 0xD0) >> 4);
        w = -(func_800705D8(p, buf, 6) != 0) & -500;
check:
        if (e->unk42[0] == 0) {
            v = rand() & 0x3F;
dispatch:
            switch (v) {
            case 0: case 1: case 2: case 3: case 4: case 5:
                func_801C5D24(e, 0, w);
                break;
            case 8: case 9:
                func_801C5DA0(e, 0, w);
                break;
            case 10: case 11:
                func_801C5E14(e, 0, w);
                break;
            }
        }
        break;
    case 5:
        break;
    }
    return 0;
}
