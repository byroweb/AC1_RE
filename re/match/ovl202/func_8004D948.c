/* ovl202 0x8004D948  size 44  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsSetSerialVol(M2C_UNK, s16, s16);          /* extern */

void func_8004D948(s16 arg0) {
    SsSetSerialVol(0, arg0, arg0);
}
