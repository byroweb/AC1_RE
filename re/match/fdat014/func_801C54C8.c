/* fdat014 0x801C54C8  size 88  permuter tier (tools/loop/permute.py): levers args func_800754D0 +arg0 proto, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800754D0(void *, M2C_UNK *);                       /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
extern M2C_UNK D_801C8498;

void func_801C54C8(void *arg0) {
    M2C_FIELD(arg0, s16 *, 0x148) = 0;
    M2C_FIELD(arg0, s32 *, 0x15C) = 0;
    if ((M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) && (func_800754D0(arg0, &D_801C8498) != 0)) {
        func_80078E1C(arg0);
    }
}
