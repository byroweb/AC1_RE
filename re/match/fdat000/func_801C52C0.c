/* fdat000 0x801C52C0  size 84  permuter tier (tools/loop/permute.py): levers bytes E + &D_801CAB98, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_801CAB4C();                            /* extern */
extern M2C_UNK D_801CAB98;

void func_801C52C0(void *arg0) {
    M2C_FIELD(arg0, s16 *, 0x36) = 2;
    M2C_FIELD(arg0, void **, 0x108) = (void *) (((M2C_UNK *)((u8 *)&D_801CAB98 + (M2C_FIELD(arg0, u8 *, 0x100) * 0x544))));
    func_801CAB4C();
}
