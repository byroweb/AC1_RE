/* ovl201 0x8008240C  size 196  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern M2C_UNK func_80081C5C;
extern M2C_UNK func_80082594;

void func_8008240C(void *arg0) {
    M2C_UNK *var_v0;
    u8 temp_v1;

    temp_v1 = M2C_FIELD(arg0, u8 *, 0x94);
    if (temp_v1 == 0) {
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_8005C2D0(0x18, 0x28, 0x120, 0x88, 3);
        func_8005C2D0(0x18, 0xB2, 0x90, 0x30, 5);
        var_v0 = &func_80082594;
        goto block_4;
    }
    if (temp_v1 == 1) {
        var_v0 = &func_80081C5C;
block_4:
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = var_v0;
    }
}
