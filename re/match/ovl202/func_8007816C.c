/* ovl202 0x8007816C  size 124  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800705D8(s32 *, M2C_UNK, M2C_UNK);     /* extern */

void func_8007816C(void *arg0, void *arg1, M2C_UNK arg2) {
    union { s8 b[0x20]; s32 w; } sp_frame;

    (*(s32 *)(sp_frame.b + 0x0)) = (s32) M2C_FIELD(arg0, s16 *, 8);
    (*(s32 *)(sp_frame.b + 0x8)) = (s32) M2C_FIELD(arg0, s16 *, 0xC);
    (*(s32 *)(sp_frame.b + 0x4)) = M2C_FIELD(arg0, s16 *, 0xA) - ((u16) M2C_FIELD(arg0, u16 *, 0x2A) >> 1);
    (*(s32 *)(sp_frame.b + 0xC)) = (s32) M2C_FIELD(arg1, s16 *, 8);
    (*(s32 *)(sp_frame.b + 0x14)) = (s32) M2C_FIELD(arg1, s16 *, 0xC);
    (*(s32 *)(sp_frame.b + 0x10)) = M2C_FIELD(arg1, s16 *, 0xA) - ((u16) M2C_FIELD(arg1, u16 *, 0x2A) >> 1);
    func_800705D8(&(*(s32 *)(sp_frame.b + 0x0)), arg2, 7);
}
