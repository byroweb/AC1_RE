/* ovl202 0x80082CAC  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s8 D_801987D1;
extern s8 D_801987D2;
extern s32 D_801987D4;
extern s16 D_801987DA;
extern s16 D_801987DC;
extern s16 D_801987DE;
extern s16 D_801987E0;
extern s32 D_801987E4;

void func_80082CAC(s16 arg0, s16 arg1, s16 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7) {
    D_801987DA = arg0;
    D_801987DC = arg1;
    D_801987E0 = arg2;
    D_801987E4 = arg3;
    D_801987DE = (s16) arg4;
    D_801987D1 = (s8) arg5;
    D_801987D2 = (s8) arg6;
    D_801987D4 = arg7;
}
