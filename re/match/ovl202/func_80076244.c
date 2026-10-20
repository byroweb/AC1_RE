/* ovl202 0x80076244  size 104  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004EC4C(s32);                         /* extern */
M2C_UNK func_80075FC0();                            /* extern */

void func_80076244(void *arg0) {
    M2C_UNK (*temp_v0_2)(void *, u8);
    u8 temp_v0;

    func_80075FC0();
    func_8004EC4C(arg0 + 0x64);
    temp_v0 = M2C_FIELD(arg0, u8 *, 0x66) - 1;
    M2C_FIELD(arg0, u8 *, 0x66) = temp_v0;
    if (!(temp_v0 & 0xFF)) {
        temp_v0_2 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
        if (temp_v0_2 != NULL) {
            temp_v0_2(arg0, M2C_FIELD(arg0, u8 *, 0x41));
        }
    }
}
