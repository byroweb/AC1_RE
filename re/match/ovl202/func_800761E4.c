/* ovl202 0x800761E4  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8004F080(void *, void *);                  /* extern */
M2C_UNK func_80075B3C(void *);                      /* extern */
M2C_UNK func_80075FC0();                            /* extern */

void func_800761E4(void *arg0) {
    M2C_UNK (*temp_v0)(void *, u8);

    func_80075FC0();
    func_80075B3C(arg0);
    if (func_8004F080(arg0 + 0x64, arg0 + 8) == 0) {
        temp_v0 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
        if (temp_v0 != NULL) {
            temp_v0(arg0, M2C_FIELD(arg0, u8 *, 0x41));
        }
    }
}
