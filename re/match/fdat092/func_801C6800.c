/* fdat092 0x801C6800  size 128  permuter tier (tools/loop/permute.py): levers index D_8019FAB9 u8[] + bytes E + &D_801A26B8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_8019F51C;
extern M2C_UNK D_801A26B8;

extern u8 D_8019FAB9[];
void func_801C6800(s32 arg0) {
    void *temp_a0;

    temp_a0 = ((M2C_UNK *)((u8 *)&D_801A26B8 + ((*(u8 *)(D_8019FAB9 + ((arg0 * 0x2C)))) * 0x170)));
    M2C_FIELD(((M2C_FIELD(temp_a0, s16 *, 0xE) * 4) + D_8019F51C), M2C_UNK (**)(void *), 0x20)(temp_a0);
}
