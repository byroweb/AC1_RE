/* exe 0x80015A08  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800158C8();                                /* extern */
M2C_UNK func_8001594C(M2C_UNK, s32);                /* extern */

void func_80015A08(s32 arg0, s32 arg1, M2C_UNK arg2) {
    func_8001594C(arg2, func_800158C8() + arg1);
}
