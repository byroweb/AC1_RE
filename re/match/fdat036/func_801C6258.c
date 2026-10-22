/* fdat036 0x801C6258  size 200  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK LoadAverageCol(M2C_UNK *, M2C_UNK *, s32, s32, void *); /* extern */
M2C_UNK func_80056B54(void *, void *, M2C_UNK, s16, s32, M2C_UNK *, s32); /* extern */
M2C_UNK func_801C6238();                            /* extern */
s32 rsin(u16);                                      /* extern */
extern M2C_UNK D_801C7348;
extern M2C_UNK D_801C734C;

void func_801C6258(void *arg0) {
    s32 temp_a2;
    s32 temp_s0;
    s32 temp_s0_2;

    func_801C6238();
    M2C_FIELD(arg0, s16 *, 0x148) = 0xF;
    M2C_FIELD(arg0, u16 *, 0xFC) = (u16) (M2C_FIELD(arg0, u16 *, 0xFC) + 0x40);
    M2C_FIELD(arg0, u16 *, 0xFE) = (u16) (M2C_FIELD(arg0, u16 *, 0xFE) + 0x30);
    temp_s0 = rsin(M2C_FIELD(arg0, u16 *, 0xFC));
    temp_s0_2 = temp_s0 + rsin(M2C_FIELD(arg0, u16 *, 0xFE)) + 0x2000;
    temp_a2 = temp_s0_2 >> 2;
    LoadAverageCol(&D_801C7348, &D_801C734C, temp_a2, 0x1000 - temp_a2, arg0 + 0x14A);
    func_80056B54(arg0, arg0 + 8, 0, M2C_FIELD(arg0, s16 *, 0x16), 0xFFFF, &D_801C734C, temp_s0_2 >> 4);
}
