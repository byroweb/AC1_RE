/* ovl202 0x8008A344  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_80073904();                                /* extern */
extern s32 D_80048614;

void func_8008A344(void) {
    s32 temp_v1;

    g_mission_ctl.unk00 = func_80073904();
    g_mission_ctl.unk02 = 0x4B0;
    temp_v1 = M2C_FIELD(g_world_structs, s32 *, 0x18000);
    g_mission_ctl.vm_set = 0xFF;
    M2C_FIELD(g_world_structs, s32 *, 0x18000) = 0;
    D_80048614 = 0;
    g_mission_ctl.heap = (u8 *) ((temp_v1 + 3) & ~3);
}
