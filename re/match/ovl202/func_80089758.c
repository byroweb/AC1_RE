/* ovl202 0x80089758  size 332  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8006D534(s32, void *, void *, void *);     /* extern */
M2C_UNK func_800835E8(s32, s32, void *, void *, void *); /* extern */
M2C_UNK func_80086854(M2C_UNK, void *, void *);     /* extern */
M2C_UNK func_800873F8(s32, void *, s32, s32, s32, s32); /* extern */

void func_80089758(void *arg0) {
    s32 temp_v0;
    u32 temp_a3;
    u32 temp_v0_2;
    void *temp_s3;
    void *temp_s4;

    if (M2C_FIELD(arg0, s16 *, 0x12) < 2) {
        M2C_FIELD(arg0, s16 *, 0x12) = 0;
        return;
    }
    temp_s4 = arg0 + 0x1E;
    temp_s3 = arg0 + 0x26;
    temp_v0 = func_8006D534(M2C_FIELD(arg0, s32 *, 8), arg0, temp_s4, temp_s3);
    if (temp_v0 != 0) {
        temp_v0_2 = M2C_FIELD(arg0, u32 *, 0x34);
        func_800873F8(temp_v0, temp_s3, temp_v0_2 & 0x3FF, (temp_v0_2 >> 0xA) & 0xFF, (temp_v0_2 >> 0x12) & 0x1F, 1);
        func_800835E8(M2C_FIELD(arg0, s32 *, 8), temp_v0, arg0 + 0x18, temp_s4, temp_s3);
        temp_a3 = M2C_FIELD(arg0, u32 *, 0x34);
        func_800873F8(M2C_FIELD(arg0, s32 *, 0x38), temp_s3, temp_a3 & 0x3FF, (temp_a3 >> 0xA) & 0xFF, (temp_a3 >> 0x12) & 0x1F, 1);
        func_80086854(0, arg0, arg0 + 0x30);
        M2C_FIELD(arg0, s16 *, 0x12) = 0;
        return;
    }
    M2C_FIELD(arg0, u16 *, 0) = (u16) (M2C_FIELD(arg0, u16 *, 0) + M2C_FIELD(arg0, u16 *, 0x1E));
    M2C_FIELD(arg0, u16 *, 2) = (u16) (M2C_FIELD(arg0, u16 *, 2) + M2C_FIELD(arg0, u16 *, 0x20));
    M2C_FIELD(arg0, u16 *, 4) = (u16) (M2C_FIELD(arg0, u16 *, 4) + M2C_FIELD(arg0, u16 *, 0x22));
    M2C_FIELD(arg0, s16 *, 0xE) = world_section_find((s32) M2C_FIELD(arg0, s16 *, 0xE), arg0);
}
