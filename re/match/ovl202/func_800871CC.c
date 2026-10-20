/* ovl202 0x800871CC  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 rand();                                         /* extern */

s32 func_800871CC(s32 arg0) {
    return ((s32) (rand() * arg0) >> 0xF) - (arg0 >> 1);
}
