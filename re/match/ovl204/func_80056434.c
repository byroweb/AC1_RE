/* ovl204 0x80056434  size 596  permuter tier (tools/loop/permute.py): levers etype D_801ADA54 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SetDefDispEnv(void *, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK SetDefDrawEnv(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK SetFarColor(M2C_UNK, M2C_UNK, M2C_UNK);     /* extern */
M2C_UNK SetGeomOffset(M2C_UNK, M2C_UNK);            /* extern */
M2C_UNK SetGeomScreen(M2C_UNK);                     /* extern */
M2C_UNK func_800154D8(M2C_UNK *, M2C_UNK *, M2C_UNK); /* extern */
M2C_UNK func_80054888();                            /* extern */
M2C_UNK func_80056180();                            /* extern */
M2C_UNK func_800562FC();                            /* extern */
M2C_UNK func_8005631C();                            /* extern */
extern M2C_UNK D_8008970C;
extern M2C_UNK D_8008CB48;
extern s32 D_801ADA48;
extern u8 D_801ADA54;
extern s8 D_801ADA6D;
extern s8 D_801ADA6E;
extern s8 D_801ADA6F;
extern s8 D_801ADAC9;
extern s8 D_801ADACA;
extern s8 D_801ADACB;
extern s8 D_801ADB25;
extern s8 D_801ADB26;
extern s8 D_801ADB27;
extern s8 D_801ADBA9;
extern s8 D_801ADBAA;
extern s8 D_801ADBAB;
extern s8 D_801ADC05;
extern s8 D_801ADC06;
extern s8 D_801ADC07;
extern s8 D_801ADC61;
extern s8 D_801ADC62;
extern s8 D_801ADC63;
extern s32 D_801B4104;
extern s32 D_801B4108;
extern s32 D_801B4114;
extern s8 D_801B4198;
extern s8 D_801B4199;
extern s8 D_801B419A;
extern s8 D_801B419B;
extern s32 D_801B4F60;
extern s32 D_801B4F74;
extern s32 D_801B4F88;

void func_80056434(void) {
    SetGeomOffset(0x4F, 0x3C);
    SetGeomScreen(0xA0);
    SetDefDrawEnv(&D_801ADA54, 2, 0x28, 0x9E, 0x78);
    SetDefDrawEnv(&D_801ADA54 + 0x5C, 0xA0, 0x28, 0x9E, 0x78);
    SetDefDrawEnv(&D_801ADA54 + 0xB8, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(&D_801ADA54 + 0x114, 0, 0xF0, 0x140, 0xF0);
    SetDefDrawEnv(&D_801ADA54 + 0x13C, 2, 0x118, 0x9E, 0x78);
    SetDefDrawEnv(&D_801ADA54 + 0x198, 0xA0, 0x118, 0x9E, 0x78);
    SetDefDrawEnv(&D_801ADA54 + 0x1F4, 0, 0xF0, 0x140, 0xF0);
    SetDefDispEnv(&D_801ADA54 + 0x250, 0, 0, 0x140, 0xF0);
    D_801ADA6D = 0;
    D_801ADA6E = 0;
    D_801ADA6F = 0;
    D_801ADBA9 = 0;
    D_801ADBAA = 0;
    D_801ADBAB = 0;
    D_801ADAC9 = 0;
    D_801ADACA = 0;
    D_801ADACB = 0;
    D_801ADC05 = 0;
    D_801ADC06 = 0;
    D_801ADC07 = 0;
    D_801ADB25 = 0;
    D_801ADB26 = 0;
    D_801ADB27 = 0;
    D_801ADC61 = 0;
    D_801ADC62 = 0;
    D_801ADC63 = 0;
    func_8005631C();
    SetGeomScreen(0xA0);
    SetFarColor(0, 0, 0);
    D_801ADA48 = -1;
    func_80054888();
    func_80056180();
    D_801B4F88 = 0;
    D_801B4F60 = 0;
    D_801B4F74 = 0;
    D_801B419B = 0;
    D_801B419A = 0;
    D_801B4199 = 0;
    D_801B4198 = 0;
    D_801B4114 = 0;
    D_801B4108 = 0;
    func_800154D8(&D_8008CB48, &D_8008970C, 0x200);
    func_800562FC();
    D_801B4104 = 1;
}
