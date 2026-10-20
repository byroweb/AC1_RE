/* ovl202 0x80062934  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80062860(M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
s32 rsin(s32);                                      /* extern */
extern s32 D_801AC81C;

void func_80062934(void) {
    func_80062860(0, 1, 8, (s32) (rsin(D_801AC81C << 9) + 0x1400) >> 1);
}
