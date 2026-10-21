#include "ac1/ac1.h"

extern s32 func_8005360C(s32, s32, s32, s32);
extern void func_80053688(s32, s32);

s32 func_800537FC(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 r;

    r = func_8005360C(a0, a1, a2, a4);
    func_80053688(a3, a4);
    return r;
}
