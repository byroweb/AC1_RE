/* fdat092 0x801C67A4  size 92  permuter tier (tools/loop/permute.py): levers index D_8019FAB9 u8[] + bytes E + &D_801A26B8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK D_801A26B8;

extern u8 D_8019FAB9[];
void func_801C67A4(s32 arg0) {
    void *temp_v0;

    temp_v0 = ((M2C_UNK *)((u8 *)&D_801A26B8 + ((*(u8 *)(D_8019FAB9 + ((arg0 * 0x2C)))) * 0x170)));
    M2C_FIELD(temp_v0, s32 *, 0x4C) = 0;
    M2C_FIELD(temp_v0, s32 *, 0x50) = 0;
    M2C_FIELD(temp_v0, s32 *, 0x54) = 0;
    M2C_FIELD(temp_v0, s32 *, 0x58) = 0;
    M2C_FIELD(temp_v0, s32 *, 0x5C) = 0;
}
