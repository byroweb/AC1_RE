/* ovl201 0x800B2F18  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80015B78(M2C_UNK, M2C_UNK *, s32, s32, s32); /* extern */
s32 func_80015C9C(M2C_UNK, s32, M2C_UNK *);         /* extern */
s32 func_80050FC4(s32);                             /* extern */

s32 func_800B2F18(s32 arg0) {
    M2C_UNK sp18;
    s32 temp_v0;
    s32 temp_v0_2;

    temp_v0_2 = func_80015C9C(4, arg0 & 0xFFFF, &sp18);
    temp_v0 = func_80050FC4(temp_v0_2);
    func_80015B78(0x10, &sp18, temp_v0_2, temp_v0, 0);
    return temp_v0;
}
