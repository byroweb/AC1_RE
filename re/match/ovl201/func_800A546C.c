/* ovl201 0x800A546C  size 572  permuter tier (tools/loop/permute.py): levers etype D_8017F450 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005CB94(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_800A56A8(M2C_UNK *, u16);              /* extern */
M2C_UNK func_800A79B0();                            /* extern */
extern u16 D_80039D08;
extern u16 D_80039D0A;
extern M2C_UNK D_8004D23C;
extern M2C_UNK D_8004D248;
extern M2C_UNK D_8004D254;
extern M2C_UNK D_8004D264;
extern M2C_UNK D_8004D274;
extern M2C_UNK D_8004D280;
extern M2C_UNK D_8004D288;
extern M2C_UNK D_8004D290;
extern M2C_UNK D_800BDCD8;
extern u8 D_8017F450;

void func_800A546C(void) {
    M2C_UNK *temp_s1;

    func_800A79B0();
    func_8005D754(3, &D_800BDCD8, 0, 0, 4);
    func_8005D754(3, &D_800BDCD8, 0, 0x18, 4);
    func_8005D754(3, &D_800BDCD8, 0, 0x30, 4);
    func_8005D754(3, &D_800BDCD8, 0, 0x48, 4);
    func_8005D754(3, &D_800BDCD8, 0, 0x60, 4);
    func_8005D754(3, &D_800BDCD8, 0, 0x78, 4);
    func_8005D80C(2, &D_8004D23C, 0x10, 8, 4, 4);
    func_8005D80C(2, &D_8004D248, 0x10, 8, 0x1C, 4);
    func_8005D80C(2, &D_8004D254, 0x10, 8, 0x34, 4);
    func_8005D80C(2, &D_8004D264, 0x10, 8, 0x4C, 4);
    func_8005D80C(2, &D_8004D274, 0x10, 8, 0x64, 4);
    func_8005D80C(2, &D_8004D280, 0x10, 8, 0x7C, 4);
    func_8005CB94(&D_8004D288, 0x10, 4, 6);
    func_8005CB94(&D_8004D290, 0x50, 4, 6);
    func_800A56A8(&D_8017F450, D_80039D08);
    temp_s1 = &D_8017F450 + 4;
    func_800A56A8(temp_s1, D_80039D0A);
    func_8005D80C(2, &D_8017F450, 0x10, 0x1C, 0x1C, 6);
    func_8005D80C(2, temp_s1, 0x10, 0x64, 0x1C, 6);
}
