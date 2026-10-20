/* ovl201 0x8006FE38  size 32  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8006FE38(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x48) = (s32) ((M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), u8 *, 0x86) * 8) + 0xA8);
}
