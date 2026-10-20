/* ovl202 0x800760DC  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8004F024(s32, s32);                        /* extern */
M2C_UNK func_80075FC0();                            /* extern */

void func_800760DC(void *arg0) {
    M2C_UNK (*temp_v0)(void *, u8);

    func_80075FC0();
    if (func_8004F024(arg0 + 0x64, arg0 + 8) == 0) {
        temp_v0 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
        if (temp_v0 != NULL) {
            temp_v0(arg0, M2C_FIELD(arg0, u8 *, 0x41));
        }
    }
}
