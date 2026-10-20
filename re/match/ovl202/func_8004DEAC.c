/* ovl202 0x8004DEAC  size 168  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800131D0(void *, u8, s16);             /* extern */

void func_8004DEAC(void *arg0, u8 arg1, void *arg2, void *arg3) {
    M2C_FIELD(arg0, u8 *, 2) = arg1;
    M2C_FIELD(arg0, u8 *, 3) = arg1;
    func_800131D0(arg0 + 0x20, arg1 & 0xFF, M2C_FIELD(arg2, s16 *, 0));
    func_800131D0(arg0 + 0x28, M2C_FIELD(arg0, u8 *, 3), M2C_FIELD(arg2, s16 *, 2));
    func_800131D0(arg0 + 0x30, M2C_FIELD(arg0, u8 *, 3), M2C_FIELD(arg2, s16 *, 4));
    func_800131D0(arg0 + 0x38, M2C_FIELD(arg0, u8 *, 3), M2C_FIELD(arg3, s16 *, 0));
    func_800131D0(arg0 + 0x40, M2C_FIELD(arg0, u8 *, 3), M2C_FIELD(arg3, s16 *, 2));
    func_800131D0(arg0 + 0x48, M2C_FIELD(arg0, u8 *, 3), M2C_FIELD(arg3, s16 *, 4));
}
