/* ovl202 0x8006E330  size 44  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8006E330(s32 arg0, s32 arg1) {
    s32 *temp_a2;

    temp_a2 = ((arg1 >> 5) * 4) + arg0;
    *temp_a2 &= ~(1 << (arg1 & 0x1F));
}
