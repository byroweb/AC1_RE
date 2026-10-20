/* ovl201 0x8008630C  size 232  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern s16 D_801A2318;
extern s32 D_801A2550;
extern s32 D_801A2568;
extern M2C_UNK func_800863F4;

void func_8008630C(void *arg0) {
    if (D_801A2568 & 0x60) {
        D_801A2550 |= D_801A2568;
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        func_800506EC(4);
        func_800506EC(8);
        obj_find_by_id(3)->kill = 1;
        obj_find_by_id(7)->kill = 1;
        func_8005C2D0(8, 0x28, 0xA0, 0xB4, 3);
        func_8005C2D0(0xB0, 0xA2, 0x80, 0x3E, 7);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800863F4;
    }
}
