#include "ac1/ac1.h"

extern s32 func_80014DFC();
extern s32 func_80073714();
extern void func_800759A4();
extern u8 D_80039C78[];
extern u8 D_801C7530[];

s32 func_801C5E40(s32 a0, s32 a1, ...)
{
    s32 t;
    s32 v;
    u8 a;

    t = func_80073714(a0, 0x7FFFFFFF);
    if (a1 == 5) {
        return 0;
    }
    if (a1 < 6) {
        if (a1 != 0) {
            return 0;
        }
        if (t >= 0x1389) {
            func_800759A4(a0, 1, D_80039C78);
        } else {
            v = func_80014DFC(4, D_801C7530, 1);
            if (v != 0) {
                func_800759A4(a0, D_801C7530[v + 4], D_80039C78);
            }
        }
    }
    return 0;
}
