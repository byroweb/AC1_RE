/* ovl201 0x80051450  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80015B78(M2C_UNK, M2C_UNK *, s32, s32, M2C_UNK *); /* extern */
s32 func_80015C9C(s32, s32, M2C_UNK *);             /* extern */
s32 func_80050FC4(s32);                             /* extern */
extern M2C_UNK func_80051830;

void func_80051450(s32 arg0, s32 arg1) {
    M2C_UNK sp18;
    s32 temp_v0;

    temp_v0 = func_80015C9C(arg0 & 0xFFFF, arg1 & 0xFFFF, &sp18);
    func_80015B78(0x10, &sp18, temp_v0, func_80050FC4(temp_v0 + 4), &func_80051830);
}
