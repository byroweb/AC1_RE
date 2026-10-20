/* ovl202 0x8007A19C  size 124  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004CC30(M2C_UNK, s32, void *, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8004CE44(M2C_UNK, void *, M2C_UNK, M2C_UNK); /* extern */
extern s32 D_801A5DC0;

void func_8007A19C(void *arg0) {
    if (!(M2C_FIELD(arg0, u16 *, 0x148) & 0xF)) {
        func_8004CE44(1, arg0 + 8, 0x50, -0x50);
        return;
    }
    func_8004CC30(0xB, D_801A5DC0 + 0x1000, arg0 + 8, 0x50, 0x1F40, 0x2710, -0x30);
}
