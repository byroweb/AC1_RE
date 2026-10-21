/* fdat016 0x801C6A60  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_801C69BC();                            /* extern */
extern M2C_UNK func_801C66DC;
extern M2C_UNK func_801C6904;

void func_801C6A60(void *arg0) {
    func_801C69BC();
    M2C_FIELD(arg0, M2C_UNK **, 0x58) = &func_801C66DC;
    M2C_FIELD(arg0, M2C_UNK **, 0x5C) = &func_801C6904;
}
