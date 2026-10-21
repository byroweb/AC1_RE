/* fdat060 0x801C6B1C  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_801CC28C();                            /* extern */
extern M2C_UNK D_801CC410;

void func_801C6B1C(void *arg0) {
    M2C_FIELD(arg0, s16 *, 0x36) = 2;
    M2C_FIELD(arg0, M2C_UNK **, 0x108) = &D_801CC410;
    func_801CC28C();
    M2C_FIELD(arg0, u16 *, 0x164) = (u16) (M2C_FIELD(arg0, u16 *, 0x164) * 4);
}
