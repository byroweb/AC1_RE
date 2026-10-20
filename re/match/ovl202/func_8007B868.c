/* ovl202 0x8007B868  size 128  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8004F080(void *, void *);                  /* extern */

void func_8007B868(void *arg0) {
    M2C_UNK (*temp_v0_2)(void *, u8);
    s16 temp_v0;

    temp_v0 = M2C_FIELD(arg0, s16 *, 0xD2);
    if (temp_v0 == 0) {
        M2C_FIELD(arg0, u16 *, 0x36) = (u16) (M2C_FIELD(arg0, u16 *, 0x36) & 0xFFEF);
    } else {
        M2C_FIELD(arg0, s16 *, 0xD2) = (s16) (temp_v0 - 1);
    }
    if (func_8004F080(arg0 + 0x64, arg0 + 8) == 0) {
        temp_v0_2 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
        if (temp_v0_2 != NULL) {
            temp_v0_2(arg0, M2C_FIELD(arg0, u8 *, 0x41));
        }
    }
}
