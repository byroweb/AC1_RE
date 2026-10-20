/* ovl201 0x8008F2D0  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK func_8005D15C(M2C_UNK *, M2C_UNK, s32);     /* extern */
M2C_UNK func_8008EC30(s32, M2C_UNK *);              /* extern */
s32 func_8008F08C(s32, s32);                        /* extern */
extern M2C_UNK D_8004CD18;

void func_8008F2D0(s32 arg0, s32 arg1, s32 arg2) {
    M2C_UNK sp10;
    s32 temp_s0;

    func_8008EC30(func_8008F08C(arg0 & 0xFF, arg1 & 0xFF), &sp10);
    temp_s0 = ((arg2 & 0xFF) * 0x10) + 0x80;
    func_8005D15C(&D_8004CD18, 0x3EC, temp_s0);
    func_8005D15C(&sp10, 0x3EC, temp_s0);
    DrawSync(0);
}
