/* ovl202 0x80073924  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8001628C();                                /* extern */

s32 func_80073924(s32 arg0) {
    return (func_8001628C() & 0xFFFF) - (arg0 & 0xFFFF);
}
