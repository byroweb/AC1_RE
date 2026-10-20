/* ovl201 0x800A9740  size 760  permuter tier (tools/loop/permute.py): levers etype D_8017F4C8 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_800A9A38(M2C_UNK *, u16);              /* extern */
M2C_UNK func_800ADB40(s32, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
extern u16 D_80039D08;
extern u16 D_80039D0A;
extern u16 D_80039D0C;
extern u16 D_80039D0E;
extern M2C_UNK D_8004D3A0;
extern M2C_UNK D_8004D3AC;
extern M2C_UNK D_8004D3B8;
extern M2C_UNK D_8004D3C4;
extern M2C_UNK D_8004D3D4;
extern M2C_UNK D_8004D3D8;
extern M2C_UNK D_8004D3DC;
extern M2C_UNK D_8004D3E0;
extern M2C_UNK D_800BDD34;
extern M2C_UNK D_800BDD64;
extern u8 D_8017F4C8;

void func_800A9740(s32 arg0) {
    M2C_UNK *temp_s2;
    M2C_UNK *temp_s3;
    M2C_UNK *temp_s4;

    func_800ADB40(arg0 + 0x7C, 0, 0, 4);
    func_8005D754(3, &D_800BDD34, 0, 0, 4);
    func_8005D754(3, &D_800BDD34, 0, 0x18, 4);
    func_8005D754(3, &D_800BDD34, 0, 0x30, 4);
    func_8005D754(3, &D_800BDD34, 0, 0x48, 4);
    func_8005D754(1, &D_800BDD64, 4, 2, 8);
    func_8005D80C(2, &D_8004D3A0, 0x10, 8, 4, 4);
    func_8005D80C(2, &D_8004D3AC, 0x10, 8, 0x1C, 4);
    func_8005D80C(2, &D_8004D3B8, 0x10, 8, 0x34, 4);
    func_8005D80C(2, &D_8004D3C4, 0x10, 8, 0x4C, 4);
    func_8005D80C(2, &D_8004D3D4, 0x10, 0x20, 4, 6);
    func_8005D80C(2, &D_8004D3D8, 0x10, 0x68, 4, 6);
    func_8005D80C(2, &D_8004D3DC, 8, 7, 0x1C, 6);
    func_8005D80C(2, &D_8004D3E0, 8, 0x27, 0x1C, 6);
    func_8005D80C(2, &D_8004D3DC, 8, 0x50, 0x1C, 6);
    func_8005D80C(2, &D_8004D3E0, 8, 0x70, 0x1C, 6);
    func_800A9A38(&D_8017F4C8, D_80039D08);
    temp_s2 = &D_8017F4C8 + 4;
    func_800A9A38(temp_s2, D_80039D0A);
    temp_s3 = &D_8017F4C8 + 8;
    func_800A9A38(temp_s3, D_80039D0C);
    temp_s4 = &D_8017F4C8 + 0xC;
    func_800A9A38(temp_s4, D_80039D0E);
    func_8005D80C(2, &D_8017F4C8, 8, 7, 0x2C, 6);
    func_8005D80C(2, temp_s2, 8, 0x2F, 0x2C, 6);
    func_8005D80C(2, temp_s3, 8, 0x50, 0x2C, 6);
    func_8005D80C(2, temp_s4, 8, 0x78, 0x2C, 6);
}
