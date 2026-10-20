/* ovl202 0x80057360  size 376  permuter tier (tools/loop/permute.py): levers etype D_801A5E20 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SetDefDispEnv(void *, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK SetDefDrawEnv(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK SetFarColor(M2C_UNK, M2C_UNK, M2C_UNK);     /* extern */
M2C_UNK SetGeomOffset(M2C_UNK, M2C_UNK);            /* extern */
M2C_UNK SetGeomScreen(M2C_UNK);                     /* extern */
M2C_UNK func_800154D8(M2C_UNK *, M2C_UNK *, M2C_UNK); /* extern */
M2C_UNK func_8005583C();                            /* extern */
M2C_UNK func_80057134();                            /* extern */
M2C_UNK func_80057288();                            /* extern */
M2C_UNK func_800572A8();                            /* extern */
extern M2C_UNK D_8008DCA0;
extern M2C_UNK D_80090A28;
extern s32 D_801A5E18;
extern u8 D_801A5E20;
extern s8 D_801A5E39;
extern s8 D_801A5E3A;
extern s8 D_801A5E3B;
extern s8 D_801A5EB9;
extern s8 D_801A5EBA;
extern s8 D_801A5EBB;
extern s32 D_801AC80C;
extern s32 D_801AC810;
extern s32 D_801AC81C;
extern s8 D_801AC8A0;
extern s8 D_801AC8A1;
extern s8 D_801AC8A2;
extern s8 D_801AC8A3;
extern s32 D_801AD668;

void func_80057360(void) {
    SetGeomOffset(0xA0, 0x78);
    SetGeomScreen(0x140);
    SetDefDrawEnv(&D_801A5E20, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(&D_801A5E20 + 0x5C, 0, 0xF0, 0x140, 0xF0);
    SetDefDrawEnv(&D_801A5E20 + 0x80, 0, 0xF0, 0x140, 0xF0);
    SetDefDispEnv(&D_801A5E20 + 0xDC, 0, 0, 0x140, 0xF0);
    D_801A5E39 = 0;
    D_801A5E3A = 0;
    D_801A5E3B = 0;
    D_801A5EB9 = 0;
    D_801A5EBA = 0;
    D_801A5EBB = 0;
    func_800572A8();
    SetGeomScreen(0x140);
    SetFarColor(0, 0, 0);
    D_801A5E18 = -1;
    func_8005583C();
    func_80057134();
    D_801AD668 = 0;
    D_801AC8A3 = 0;
    D_801AC8A2 = 0;
    D_801AC8A1 = 0;
    D_801AC8A0 = 0;
    D_801AC81C = 0;
    D_801AC810 = 0;
    func_800154D8(&D_80090A28, &D_8008DCA0, 0x200);
    func_80057288();
    D_801AC80C = 1;
}
