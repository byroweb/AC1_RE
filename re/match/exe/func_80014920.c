/* exe 0x80014920  size 16  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014920(s32 arg0, s32 arg1) {
    return ((arg0 - arg1) & 0xFFF) < 0x801;
}
