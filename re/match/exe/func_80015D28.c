/* exe 0x80015D28  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80015B78(M2C_UNK, M2C_UNK *, s32, M2C_UNK, s32); /* extern */
s32 func_80015C9C(s32, s32, M2C_UNK *);             /* extern */

void func_80015D28(s32 arg0, s32 arg1, M2C_UNK arg2, s32 arg3) {
    M2C_UNK sp18;

    func_80015B78(0x10, &sp18, func_80015C9C(arg0 & 0xFFFF, arg1 & 0xFFFF, &sp18), arg2, arg3);
}
