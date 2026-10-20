/* ovl202 0x8008A798  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s8 D_801AC8A0;
extern s8 D_801AC8A1;
extern s8 D_801AC8A2;
extern s8 D_801AC8A3;

void func_8008A798(s32 arg0) {
    s8 temp_v1;

    if (arg0 < 0x15) {
        if (arg0 == 0x14) {
            D_801AC8A3 = 0;
            return;
        }
        D_801AC8A3 = 2;
        temp_v1 = (((0x14 - arg0) << 8) - 0xA) / 20;
        D_801AC8A2 = temp_v1;
        D_801AC8A1 = temp_v1;
        D_801AC8A0 = temp_v1;
    }
}
