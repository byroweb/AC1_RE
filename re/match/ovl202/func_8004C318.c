/* ovl202 0x8004C318  size 40  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8004C318(u32 arg0) {
    g_mission_flags = arg0;
    if (g_mission_end_timer == 0) {
        g_mission_end_timer = 0x64;
    }
}
