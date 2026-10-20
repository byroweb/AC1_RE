/* ovl202 0x80052338  size 24  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void *D_801ED844;

s32 func_80052338(void) {
    return M2C_FIELD(D_801ED844, s32 *, 0x10) == 0;
}
