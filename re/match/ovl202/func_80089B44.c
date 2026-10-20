/* ovl202 0x80089B44  size 236  variant sweep (tools/match/variants.py): old lever: swap M2C_FIELD(arg0, u8 *, 0x / M2C_FIELD(arg0, u8 *, 0x, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80089B44(void *arg0) {
    s16 temp_v1_2;
    u8 temp_v1;

    if (M2C_FIELD(arg0, s16 *, 0x2A) < 0xF) {
        M2C_FIELD(arg0, u8 *, 0xB8) = (u8) (M2C_FIELD(arg0, u8 *, 0xB8) - 8);
        M2C_FIELD(arg0, u8 *, 0xB9) = (u8) (M2C_FIELD(arg0, u8 *, 0xB9) - 8);
        M2C_FIELD(arg0, u8 *, 0xBA) = (u8) (M2C_FIELD(arg0, u8 *, 0xBA) - 8);
    } else {
        temp_v1 = M2C_FIELD(arg0, u8 *, 0xB8);
        if (temp_v1 < 0x70U) {
            M2C_FIELD(arg0, u8 *, 0xB8) = (u8) (temp_v1 + 8);
            M2C_FIELD(arg0, u8 *, 0xB9) = (u8) (M2C_FIELD(arg0, u8 *, 0xB9) + 8);
            M2C_FIELD(arg0, u8 *, 0xBA) = (u8) (M2C_FIELD(arg0, u8 *, 0xBA) + 8);
        }
    }
    M2C_FIELD(arg0, u16 *, 0) = (u16) (M2C_FIELD(arg0, u16 *, 0) + M2C_FIELD(arg0, u16 *, 0x24));
    M2C_FIELD(arg0, u16 *, 2) = (u16) (M2C_FIELD(arg0, u16 *, 2) + M2C_FIELD(arg0, u16 *, 0x26));
    M2C_FIELD(arg0, u16 *, 4) = (u16) (M2C_FIELD(arg0, u16 *, 4) + M2C_FIELD(arg0, u16 *, 0x28));
    M2C_FIELD(arg0, s16 *, 0xE) = world_section_find((s32) M2C_FIELD(arg0, s16 *, 0xE), arg0);
    temp_v1_2 = (u16) M2C_FIELD(arg0, s16 *, 0x2A) - 1;
    M2C_FIELD(arg0, s16 *, 0x2A) = temp_v1_2;
    if ((temp_v1_2 << 0x10) == 0) {
        M2C_FIELD(arg0, s16 *, 0x12) = 0;
    }
}
