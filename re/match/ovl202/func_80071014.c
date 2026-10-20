/* ovl202 0x80071014  size 372  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80070F30(s16 *, s16 *, s16 *, s32, s32);   /* extern */

s32 func_80071014(void *arg0, s32 arg1, s32 arg2, void *arg3, void *arg4, s32 arg5, s32 arg6) {
    union { s8 b[0x20]; s32 w; } sp_frame;
    s32 var_a0;
    u32 var_v0;
    u32 var_v0_2;
    u32 var_v0_3;

    (*(s16 *)(sp_frame.b + 0x0)) = M2C_FIELD(arg0, u16 *, 0) - arg1;
    (*(s16 *)(sp_frame.b + 0x8)) = M2C_FIELD(arg0, u16 *, 0) + arg1;
    (*(s16 *)(sp_frame.b + 0x4)) = M2C_FIELD(arg0, u16 *, 4) - arg1;
    (*(s16 *)(sp_frame.b + 0xC)) = M2C_FIELD(arg0, u16 *, 4) + arg1;
    (*(s16 *)(sp_frame.b + 0x2)) = M2C_FIELD(arg0, u16 *, 2) - arg2;
    (*(u16 *)(sp_frame.b + 0xA)) = M2C_FIELD(arg0, u16 *, 2);
    if (arg3 != NULL) {
        (*(s16 *)(sp_frame.b + 0x10)) = M2C_FIELD(arg3, u16 *, 0) * 4;
        (*(s16 *)(sp_frame.b + 0x12)) = M2C_FIELD(arg3, u16 *, 2) * 4;
        (*(s16 *)(sp_frame.b + 0x14)) = M2C_FIELD(arg3, u16 *, 4) * 4;
        var_a0 = func_80070F30(&(*(s16 *)(sp_frame.b + 0x0)), &(*(s16 *)(sp_frame.b + 0x10)), &(*(s16 *)(sp_frame.b + 0x18)), arg5, arg6);
        if ((*(s16 *)(sp_frame.b + 0x18)) >= 0) {
            var_v0 = (u32) ((*(s16 *)(sp_frame.b + 0x18)) + 3) >> 2;
        } else {
            var_v0 = (u32) -((s32) (-(*(s16 *)(sp_frame.b + 0x18)) + 3) >> 2);
        }
        M2C_FIELD(arg4, s16 *, 0) = (s16) var_v0;
        if ((*(s16 *)(sp_frame.b + 0x1A)) >= 0) {
            var_v0_2 = (u32) ((*(s16 *)(sp_frame.b + 0x1A)) + 3) >> 2;
        } else {
            var_v0_2 = (u32) -((s32) (-(*(s16 *)(sp_frame.b + 0x1A)) + 3) >> 2);
        }
        M2C_FIELD(arg4, s16 *, 2) = (s16) var_v0_2;
        if ((*(s16 *)(sp_frame.b + 0x1C)) >= 0) {
            var_v0_3 = (u32) ((*(s16 *)(sp_frame.b + 0x1C)) + 3) >> 2;
        } else {
            var_v0_3 = (u32) -((s32) (-(*(s16 *)(sp_frame.b + 0x1C)) + 3) >> 2);
        }
        M2C_FIELD(arg4, s16 *, 4) = (s16) var_v0_3;
    } else {
        var_a0 = func_80070F30(&(*(s16 *)(sp_frame.b + 0x0)), NULL, NULL, arg5, arg6);
    }
    return var_a0;
}
