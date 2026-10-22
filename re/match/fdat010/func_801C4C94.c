/* fdat010 0x801C4C94  size 156  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004CC30(M2C_UNK, s32, void *, M2C_UNK, s32, s32, s32); /* extern */
s32 func_8004F024(void *, void *);                  /* extern */
M2C_UNK func_80075FC0(void *);                      /* extern */
extern s32 D_801A5DC0;

void func_801C4C94(void *arg0) {
    M2C_UNK (*temp_v0)(void *, u8);

    if (M2C_FIELD(arg0, u8 *, 0x41) == M2C_FIELD(arg0, u8 *, 0x3C)) {
        func_8004CC30(0x8F, D_801A5DC0 + 0x1000, arg0 + 8, 0x50, 0xBB8, 0xDAC, 0);
    }
    func_80075FC0(arg0);
    if (func_8004F024(arg0 + 0x64, arg0 + 8) == 0) {
        temp_v0 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
        if (temp_v0 != NULL) {
            temp_v0(arg0, M2C_FIELD(arg0, u8 *, 0x41));
        }
    }
}
