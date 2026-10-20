/* ovl204 0x80077EAC  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800815DC(u16, u16, u16, s32, s32, s32, s32, void *); /* extern */
extern void *D_80039D18;

void func_80077EAC(void) {
    u16 temp_a2;

    temp_a2 = M2C_FIELD(D_80039D18, u16 *, 0x28C);
    func_800815DC(M2C_FIELD(D_80039D18, u16 *, 0x288), M2C_FIELD(D_80039D18, u16 *, 0x28A), temp_a2, temp_a2 * temp_a2, (s32) M2C_FIELD(D_80039D18, u16 *, 0x28E), (s32) M2C_FIELD(D_80039D18, u8 *, 0x2AC), (s32) M2C_FIELD(D_80039D18, u8 *, 0x2AD), D_80039D18 + 0x2C0);
}
