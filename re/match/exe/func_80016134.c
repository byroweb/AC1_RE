/* exe 0x80016134  size 8  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80016134(void *arg0, s8 arg1) {
    M2C_FIELD(arg0, s8 *, -0x10) = arg1;
}
