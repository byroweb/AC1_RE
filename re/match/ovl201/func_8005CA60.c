/* ovl201 0x8005CA60  size 128  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005CAE0(M2C_UNK *, s16, s16, s16);    /* extern */
extern M2C_UNK D_800B73E8;
extern M2C_UNK D_800B73F4;

void func_8005CA60(s16 arg0, s16 arg1, s16 arg2) {
    func_8005CAE0(&D_800B73E8, arg0, arg1, arg2);
    func_8005CAE0(&D_800B73F4, (s16) (arg0 + 0x50), arg1, arg2);
}
