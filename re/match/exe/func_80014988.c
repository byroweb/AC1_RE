/* exe 0x80014988  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SquareRoot0(s32);                           /* extern */

void func_80014988(s32 arg0, s32 arg1, s32 arg2) {
    SquareRoot0((arg0 * arg0) + (arg1 * arg1) + (arg2 * arg2));
}
