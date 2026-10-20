/* ovl202 0x80052F90  size 144  permuter tier (tools/loop/permute.py): levers index D_801ED854 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u8 D_80039D10;

extern u8 D_801ED854[];
void func_80052F90(void *arg0, void *arg1) {
    u16 temp_v0;
    u16 temp_v0_2;
    u16 temp_v0_3;
    u16 temp_v0_4;

    if ((D_80039D10 != 0) || (M2C_FIELD(arg0, u8 *, 0xC) & 4)) {
        M2C_FIELD(arg1, s32 *, 4) = (s32) (*(s32 *)(D_801ED854 + ((M2C_FIELD(arg0, u8 *, 0xE) * 4))));
        temp_v0 = M2C_FIELD(arg0, u16 *, 0);
        M2C_FIELD(arg1, u16 *, 0x10) = temp_v0;
        M2C_FIELD(arg1, u16 *, 8) = temp_v0;
        temp_v0_2 = M2C_FIELD(arg0, u16 *, 2);
        M2C_FIELD(arg1, u16 *, 0xE) = temp_v0_2;
        M2C_FIELD(arg1, u16 *, 0xA) = temp_v0_2;
        temp_v0_3 = M2C_FIELD(arg0, u16 *, 4);
        M2C_FIELD(arg1, u16 *, 0x14) = temp_v0_3;
        M2C_FIELD(arg1, u16 *, 0xC) = temp_v0_3;
        temp_v0_4 = M2C_FIELD(arg0, u16 *, 6);
        M2C_FIELD(arg1, u16 *, 0x16) = temp_v0_4;
        M2C_FIELD(arg1, u16 *, 0x12) = temp_v0_4;
    }
}
