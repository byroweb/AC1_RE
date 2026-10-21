/* exe 0x80016148  size 8  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80016148(void *arg0, s8 arg1) {
    M2C_FIELD(arg0, s8 *, -0xF) = arg1;
}
