/* ovl201 0x80050BBC  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 SsVabOpenHead(s32, M2C_UNK);                    /* extern */
M2C_UNK SsVabTransBody(s32, s16);                   /* extern */
M2C_UNK SsVabTransCompleted(M2C_UNK);               /* extern */
s32 func_80051248(M2C_UNK, M2C_UNK);                /* extern */
extern s16 D_801A2318;

void func_80050BBC(void) {
    D_801A2318 = SsVabOpenHead(func_80051248(6, 0), -1);
    SsVabTransBody(func_80051248(6, 1), D_801A2318);
    SsVabTransCompleted(1);
}
