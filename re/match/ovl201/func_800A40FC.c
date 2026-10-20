/* ovl201 0x800A40FC  size 156  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern s16 D_801A2318;
extern s32 D_801A2568;
extern M2C_UNK func_800A4198;

void func_800A40FC(void *arg0) {
    if (D_801A2568 & 0x840) {
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(0x20, 0x18, 0x108, 0xC8, 3);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A4198;
    }
}
