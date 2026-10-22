/* fdat008 0x801C6D74  size 276  permuter tier (tools/loop/permute.py): levers bytes E + &D_80198B08, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_80073674(M2C_UNK);                       /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK D_80198B08;
extern M2C_UNK func_801C6870;
extern M2C_UNK func_801C6884;
extern M2C_UNK func_801C6AB0;
extern M2C_UNK func_801C6BBC;

void func_801C6D74(void *arg0) {
    u16 temp_v0_2;
    u8 temp_v1;
    void *temp_v0;

    temp_v0 = func_80073674(8);
    M2C_FIELD(arg0, void **, 0x14) = temp_v0;
    M2C_FIELD(temp_v0, M2C_UNK **, 4) = &func_801C6BBC;
    temp_v1 = M2C_FIELD(arg0, u8 *, 0x23);
    switch (temp_v1) {                              /* irregular */
    case 1:
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x14), M2C_UNK **, 0) = &func_801C6870;
        break;
    case 2:
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x14), M2C_UNK **, 0) = &func_801C6884;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x14), u32 *, 0xC) = (u32) (((M2C_UNK *)((u8 *)&D_80198B08 + (M2C_FIELD(arg0, u16 *, 0x28) * 0x34))));
        break;
    case 3:
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x14), M2C_UNK **, 0) = &func_801C6AB0;
        temp_v0_2 = M2C_FIELD(arg0, u16 *, 0x28);
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x14), u32 *, 0xC) = (u32) ((u32) (temp_v0_2 * temp_v0_2) >> 4);
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x14), s8 *, 8) = (s8) (rand() & 0x1F);
        break;
    }
    M2C_FIELD(M2C_FIELD(arg0, void **, 0x14), s8 *, 9) = 0;
}
