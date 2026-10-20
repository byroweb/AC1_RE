/* ovl202 0x80076134  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8004F0D4(s32, s32, s32);                   /* extern */

void func_80076134(void *arg0) {
    M2C_UNK (*temp_v0)(void *, u8);

    if (func_8004F0D4(arg0 + 0x64, arg0 + 8, arg0 + 0x10) == 0) {
        temp_v0 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
        if (temp_v0 != NULL) {
            temp_v0(arg0, M2C_FIELD(arg0, u8 *, 0x41));
        }
    }
}
