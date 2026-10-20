/* ovl202 0x8006E35C  size 36  zero-agent sweep (tools/loop/sweep.py): m2c ctx-fix draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

struct _m2c_stack_func_8006E35C {};                 /* size 0x0 */

s32 func_8006E35C(s32 arg0, s32 arg1) {
    return *(M2C_UNK *)(((arg1 >> 5) * 4) + arg0) & (1 << (arg1 & 0x1F));
}
