#include "ac1/ac1.h"

extern s32 func_80014DFC();
extern s32 func_80073714();
extern void func_80073C2C();
extern void func_800759A4();
extern u8 D_80039C78[];
extern u8 D_801CC400[];
extern u8 D_801CC408[];

s32 func_801C68F8(AcEntity *e, s32 msg, ...)
{
    s32 t;
    s16 sp30[4];
    s16 sp38[0x18];

    t = func_80073714(e, 0x7FFFFFFF);
    if (msg == 5) {
        return 0;
    }
    if (msg < 6) {
        if (msg != 0) {
            return 0;
        }
        func_80073C2C(e->target, sp30, *(s16 *)((u8 *)e + 0x12), 0, 0, e->faction, 0xBB8, -0x258, 0x258, -0x640, 0x640, sp38);
        if (t < 0x9C4) {
            func_800759A4(e, D_801CC400[func_80014DFC(3, D_801CC400, 1) + 4], D_80039C78);
        } else if (t >= 0xBB9) {
            func_800759A4(e, D_801CC408[func_80014DFC(3, D_801CC408, 1) + 4], D_80039C78);
        }
    }
    return 0;
}
