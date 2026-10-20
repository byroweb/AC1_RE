/* ovl202 0x800895B0  size 120  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80089484(void *, void *, s16, s16, s32, s32); /* extern */

void func_800895B0(void *arg0) {
    u16 temp_v0;

    temp_v0 = M2C_FIELD(arg0, u16 *, 0x2A) - 1;
    M2C_FIELD(arg0, u16 *, 0x2A) = temp_v0;
    if ((temp_v0 << 0x10) == 0) {
        M2C_FIELD(arg0, s16 *, 0x12) = 0;
    }
    M2C_FIELD(arg0, s16 *, 0xE) = world_section_find((s32) M2C_FIELD(arg0, s16 *, 0xE), arg0);
    func_80089484(arg0, arg0 + 0x24, M2C_FIELD(arg0, s16 *, 0x1A), M2C_FIELD(arg0, s16 *, 0x1C), (s32) M2C_FIELD(arg0, s16 *, 0x1E), (s32) M2C_FIELD(arg0, s16 *, 0x20));
}
