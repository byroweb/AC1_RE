/* ovl201 0x8007002C  size 48  permuter tier (tools/loop/permute.py): levers etype D_800B7E5C M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u8 D_800B7E5C;

void func_8007002C(void *arg0) {
    M2C_FIELD(arg0, void **, 0xC) = (void *) ((M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), u8 *, 0x8C) * 0xC) + &D_800B7E5C);
}
