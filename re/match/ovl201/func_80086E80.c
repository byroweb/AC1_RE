/* ovl201 0x80086E80  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800513F0(M2C_UNK, M2C_UNK, M2C_UNK *); /* extern */
s32 func_800517C8();                                /* extern */
extern s8 D_801F37C8;
extern M2C_UNK func_80086E34;

void func_80086E80(void) {
    do {

    } while ((func_800517C8() << 0x10) != 0);
    D_801F37C8 = 1;
    func_800513F0(0x13, 0, &func_80086E34);
}
