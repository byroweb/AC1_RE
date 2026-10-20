/* ovl201 0x8008878C  size 44  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051078(s32);                         /* extern */
extern s32 D_801F5094;

void func_8008878C(void) {
    if (D_801F5094 != 0) {
        func_80051078(D_801F5094);
    }
}
