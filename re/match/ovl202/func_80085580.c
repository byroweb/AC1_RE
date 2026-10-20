/* ovl202 0x80085580  size 484  permuter tier (tools/loop/permute.py): levers etype D_80090364 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK LoadAverageCol(void *, void *, s32, s32, M2C_UNK *); /* extern */
M2C_UNK func_80056B54(M2C_UNK, void *, M2C_UNK, s16, s32, M2C_UNK *, s32); /* extern */
s32 func_80083738(void *, s32, s32, s32, void *, void *, void *, s32); /* extern */
void *func_8008589C(M2C_UNK, void *, s32, s32, s32, s32); /* extern */
extern u8 D_80090364;

void func_80085580(void *arg0) {
    M2C_UNK sp20;
    s32 temp_a1;
    s32 temp_lo;
    u16 temp_a2;
    void *temp_s2;

    M2C_FIELD(arg0, u16 *, 0xC4) = (u16) (M2C_FIELD(arg0, u16 *, 0xC4) + M2C_FIELD(arg0, u16 *, 0x24));
    M2C_FIELD(arg0, u16 *, 0xC6) = (u16) (M2C_FIELD(arg0, u16 *, 0xC6) + M2C_FIELD(arg0, u16 *, 0x24));
    M2C_FIELD(arg0, u16 *, 0xC8) = (u16) (M2C_FIELD(arg0, u16 *, 0xC8) + M2C_FIELD(arg0, u16 *, 0x24));
    temp_s2 = (M2C_FIELD(arg0, u8 *, 0x2D) * 0x30) + &D_80090364;
    M2C_FIELD(arg0, u8 *, 0xCC) = (u8) (M2C_FIELD(arg0, u8 *, 0xCC) + M2C_FIELD(temp_s2, u8 *, 0x26));
    M2C_FIELD(arg0, u8 *, 0xCD) = (u8) (M2C_FIELD(arg0, u8 *, 0xCD) + M2C_FIELD(temp_s2, u8 *, 0x27));
    temp_a2 = M2C_FIELD(arg0, u16 *, 0xC4);
    M2C_FIELD(arg0, u8 *, 0xCE) = (u8) (M2C_FIELD(arg0, u8 *, 0xCE) + M2C_FIELD(temp_s2, u8 *, 0x28));
    M2C_FIELD(arg0, u16 *, 0x24) = (u16) (M2C_FIELD(arg0, u16 *, 0x24) + M2C_FIELD(temp_s2, u16 *, 6));
    temp_a1 = M2C_FIELD(arg0, u16 *, 0x2A) << 0x10;
    M2C_FIELD(arg0, u16 *, 0x2A) = temp_a2;
    if (M2C_FIELD(arg0, u8 *, 0x18) != 0) {
        M2C_FIELD(arg0, s32 *, 0x20) = func_80083738(arg0, temp_a1 >> 0x12, (s32) (temp_a2 << 0x10) >> 0x12, (s32) (M2C_FIELD(temp_s2, u16 *, 8) << 0x10) >> 0x12, arg0 + 0x18, temp_s2 + 0xC, temp_s2 + 0x10, M2C_FIELD(arg0, s32 *, 0x20));
    }
    if (M2C_FIELD(arg0, s16 *, 0x26) > 0) {
        if (M2C_FIELD(temp_s2, u8 *, 0x1D) != 0xFF) {
            temp_lo = (s32) (M2C_FIELD(arg0, s16 *, 0x12) << 0xC) / (s32) M2C_FIELD(arg0, u8 *, 0x2C);
            LoadAverageCol(temp_s2 + 0x1A, temp_s2 + 0x1E, temp_lo, 0x1000 - temp_lo, &sp20);
            func_80056B54(0, arg0, 0, M2C_FIELD(arg0, s16 *, 0xE), 0xFFFF, &sp20, (s32) M2C_FIELD(arg0, s16 *, 0x26));
        }
        M2C_FIELD(arg0, s16 *, 0x26) = (s16) ((u16) M2C_FIELD(arg0, s16 *, 0x26) + M2C_FIELD(arg0, u16 *, 0x28));
        M2C_FIELD(arg0, u16 *, 0x28) = (u16) (M2C_FIELD(arg0, u16 *, 0x28) + M2C_FIELD(temp_s2, u16 *, 0x18));
        M2C_FIELD(func_8008589C(0, arg0, M2C_FIELD(temp_s2, s32 *, 0x2C), (s32) ((u16) M2C_FIELD(arg0, s16 *, 0x26) << 0x10) >> 0x11, 0, (s32) M2C_FIELD(arg0, s16 *, 0xE)), s16 *, 0x12) = 0;
    }
}
