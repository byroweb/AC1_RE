/* ovl203 0x800781F0  size 104  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80081914(u16, u16, u16, s32, s32, s32, s32, void *); /* extern */
extern void *D_80039D18;
extern s32 D_80039D30;

void func_800781F0(void) {
    u16 temp_a2;

    if (D_80039D18 == D_80039D30) {
        temp_a2 = M2C_FIELD(D_80039D18, u16 *, 0x28C);
        func_80081914(M2C_FIELD(D_80039D18, u16 *, 0x288), M2C_FIELD(D_80039D18, u16 *, 0x28A), temp_a2, temp_a2 * temp_a2, (s32) M2C_FIELD(D_80039D18, u16 *, 0x28E), (s32) M2C_FIELD(D_80039D18, u8 *, 0x2AC), (s32) M2C_FIELD(D_80039D18, u8 *, 0x2AD), D_80039D18 + 0x2C0);
    }
}
