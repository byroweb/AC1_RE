/* fdat036 0x801C6618  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_801C6594();                            /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK func_801C6258;

void func_801C6618(void *arg0) {
    func_801C6594();
    M2C_FIELD(arg0, M2C_UNK **, 0x58) = &func_801C6258;
    M2C_FIELD(arg0, s16 *, 0xFC) = (s16) (rand() & 0xFFF);
    M2C_FIELD(arg0, s16 *, 0xFE) = (s16) (rand() & 0xFFF);
}
