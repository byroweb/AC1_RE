/* fdat092 0x801C69E4  size 60  permuter tier (tools/loop/permute.py): levers args func_800754D0 +arg0 proto, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800754D0(void *, M2C_UNK *);                   /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
extern M2C_UNK D_801CCBB4;

void func_801C69E4(void *arg0) {
    M2C_FIELD(arg0, s8 *, 0x40) = 0;
    func_800754D0(arg0, &D_801CCBB4);
    func_80078E1C(arg0);
}
