/* exe 0x8001514C  size 268  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014950(s32, s32);                        /* extern */
s32 func_80014F78(s32, s32, s32, s32, s32, s32 *, s32 *); /* extern */
s32 rcos(s32);                                      /* extern */
s32 rsin(s32);                                      /* extern */

s32 func_8001514C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, s16 *arg9, s16 *arg10, s16 *arg11) {
    s32 sp20;
    s32 sp24;
    s32 temp_v0;

    temp_v0 = func_80014F78(arg0, func_80014950(arg4 - arg1, arg6 - arg3), arg2 - arg5, arg7, arg8, &sp20, &sp24);
    if (temp_v0 == 0) {
        *arg10 = (s16) ((s32) (arg8 * rcos(sp24)) >> 0xC);
        *arg11 = (s16) ((s32) (arg8 * -rsin(sp24)) >> 0xC);
        *arg9 = (s16) sp20;
    }
    return temp_v0;
}
