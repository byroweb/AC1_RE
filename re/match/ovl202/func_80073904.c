/* ovl202 0x80073904  size 32  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8001628C();                                /* extern */

s32 func_80073904(void) {
    return func_8001628C() & 0xFFFF;
}
