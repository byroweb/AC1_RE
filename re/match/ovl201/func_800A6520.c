/* ovl201 0x800A6520  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern M2C_UNK func_800A6580;

void func_800A6520(void *arg0) {
    ui_pool_set_light(4, 0x10);
    func_8005BA2C(0xA0, 0x88, 0x88, 0x30, 9, 0, 0);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A6580;
}
