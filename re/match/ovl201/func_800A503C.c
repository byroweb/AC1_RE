/* ovl201 0x800A503C  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80050AF4(M2C_UNK, M2C_UNK);            /* extern */
s32 func_800517C8();                                /* extern */
M2C_UNK func_8005D39C(M2C_UNK);                     /* extern */
void *func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
extern M2C_UNK D_800BDCF0;
extern M2C_UNK func_800A50C4;

void func_800A503C(void *arg0) {
    void *temp_v0;

    if ((func_800517C8() << 0x10) == 0) {
        temp_v0 = func_8005D754(1, &D_800BDCF0, 0x26, 0x60, 2);
        M2C_FIELD(arg0, void **, 0x80) = temp_v0;
        M2C_FIELD(temp_v0, s16 *, 0x16) = 0xB;
        func_8005D39C(2);
        func_80050AF4(0xF, 1);
        func_80050AF4(0x10, 2);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A50C4;
    }
}
