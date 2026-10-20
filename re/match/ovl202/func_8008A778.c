/* ovl202 0x8008A778  size 32  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8008A778(u16 arg0, u16 arg1, u8 arg2) {
    g_mission_timer = arg0;
    g_mission_ctl.alarm_param = arg1;
    g_mission_ctl.alarm_arg = arg2;
}
