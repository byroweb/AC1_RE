/* exe 0x80014EC0  size 24  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014EC0(s32 arg0, s32 arg1, s32 arg2) {
    return ((s32) ((arg1 - arg0) * arg2) >> 0xC) + arg0;
}
