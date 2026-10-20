/* ovl202 0x8008A048  size 56  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, s32);       /* extern */

void func_8008A048(s32 arg0) {
    if ((g_mission_end_timer == 0) && (arg0 != 0)) {
        func_800529F0(0, 0, arg0);
    }
}
