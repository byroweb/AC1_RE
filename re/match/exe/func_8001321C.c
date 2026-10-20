/* exe 0x8001321C  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8001321C(s32 arg0, s32 *arg1, s32 *arg2) {
    if (arg0 < *arg1) {
        *arg1 = arg0;
        return;
    }
    if (*arg2 < arg0) {
        *arg2 = arg0;
    }
}
