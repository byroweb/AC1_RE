/* fdat022 0x801C5FD0  size 68  permuter tier (tools/loop/permute.py): levers index D_801D0B94 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80078E1C();                            /* extern */

extern u8 D_801D0B94[];
void func_801C5FD0(void *arg0) {
    if ((*(s32 *)(D_801D0B94 + ((M2C_FIELD(arg0, u16 *, 0xFC) << 6)))) == 0) {
        func_80078E1C();
    }
}
