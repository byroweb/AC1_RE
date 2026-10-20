/* ovl201 0x800B3168  size 364  permuter tier (tools/loop/permute.py): levers index D_8017F584 u8[] + index D_8017F588 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK VSync(M2C_UNK);                             /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005CB94(s32, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
extern M2C_UNK D_8004DB70;
extern M2C_UNK D_8004DB80;
extern s16 D_801A2318;
extern M2C_UNK func_800B32D4;

extern u8 D_8017F584[];
extern u8 D_8017F588[];
void func_800B3168(void *arg0) {
    s32 temp_s0;
    s32 temp_s2;

    temp_s2 = M2C_FIELD(arg0, u8 *, 0x99) + M2C_FIELD(arg0, u8 *, 0x98);
    VSync(0);
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        func_8005D80C(1, &D_8004DB70, 6, 8, 0x10, 6);
        func_8005D80C(1, &D_8004DB80, 6, 8, 0x20, 6);
        temp_s0 = temp_s2 * 0x10;
        func_8005CB94((*(s32 *)(D_8017F584 + (temp_s0))), 0x28, 8, 6);
        func_8005CB94((*(s32 *)(D_8017F588 + (temp_s0))), 0x28, 0x18, 6);
        func_8005BA2C(0xC0, 0x10, 0x78, 0x80, 7, 1, 0);
        func_8005BA2C(0x10, 0x90, 0x128, 0x50, 9, 1, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800B32D4;
    }
}
