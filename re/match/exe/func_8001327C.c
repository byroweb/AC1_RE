/* exe 0x8001327C  size 36  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8001327C(s32 arg0, s32 *arg1, s32 arg2, s32 *arg3) {
    if (arg0 < *arg1) {
        *arg1 = arg0;
        *arg3 = arg2;
    }
}
