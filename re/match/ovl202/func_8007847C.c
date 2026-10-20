/* ovl202 0x8007847C  size 124  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80086DDC(M2C_UNK, u16 *, M2C_UNK, M2C_UNK, s32, s32, s32, s32, s32, s32); /* extern */

void func_8007847C(void *arg0, M2C_UNK arg1, M2C_UNK arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7) {
    union { s8 b[0x8]; s32 w; } sp_frame;

    (*(u16 *)(sp_frame.b + 0x0)) = M2C_FIELD(arg0, u16 *, 8);
    (*(u16 *)(sp_frame.b + 0x4)) = M2C_FIELD(arg0, u16 *, 0xC);
    (*(u16 *)(sp_frame.b + 0x2)) = M2C_FIELD(arg0, u16 *, 0x2E);
    func_80086DDC(0, &(*(u16 *)(sp_frame.b + 0x0)), arg1, arg2, 0x28, arg3, arg4, arg5, arg6, arg7);
}
