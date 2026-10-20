/* ovl201 0x80051138  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 CdSearchFile(M2C_UNK *, s32);                   /* extern */
s32 func_80050FC4(u32);                             /* extern */
M2C_UNK func_800511A4(M2C_UNK *, s32, u32);         /* extern */

s32 func_80051138(s32 arg0) {
    union { s8 b[0x18]; s32 w; } sp_frame;
    s32 temp_v0;
    s32 var_v0;
    u32 temp_s0;

    var_v0 = CdSearchFile(&(*(M2C_UNK *)(sp_frame.b + 0x0)), arg0);
    if (var_v0 != 0) {
        temp_s0 = ((*(s32 *)(sp_frame.b + 0x4)) + 0x7FF) & ~0x7FF;
        temp_v0 = func_80050FC4(temp_s0);
        func_800511A4(&(*(M2C_UNK *)(sp_frame.b + 0x0)), temp_v0, temp_s0 >> 0xB);
        var_v0 = temp_v0;
    }
    return var_v0;
}
