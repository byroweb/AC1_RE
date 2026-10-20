/* ovl201 0x8008C394  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800513F0(M2C_UNK, M2C_UNK, M2C_UNK *); /* extern */
s32 func_800517C8();                                /* extern */
extern s8 D_8019C090;
extern M2C_UNK func_8008C348;

void func_8008C394(void) {
    do {

    } while ((func_800517C8() << 0x10) != 0);
    D_8019C090 = 1;
    func_800513F0(0x13, 0x331, &func_8008C348);
}
