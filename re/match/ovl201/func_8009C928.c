/* ovl201 0x8009C928  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_80039D14;

void func_8009C928(void) {
    D_80039D14 = ((D_80039D14 + 0x3FD) * 0x6D) & 0x7FFF;
}
