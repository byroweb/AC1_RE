/* fdat014 0x801C4E00  size 224  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004CE44(M2C_UNK, void *, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8004ECA8(void *);                      /* extern */
M2C_UNK func_80075FC0(void *);                      /* extern */
M2C_UNK func_80077B78(void *, M2C_UNK, u16, u8, M2C_UNK *, s32, s32); /* extern */
M2C_UNK func_801C4CB0(void *);                      /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK D_801C8478;

void func_801C4E00(void *arg0) {
    M2C_UNK (*temp_v0_3)(void *, u8);
    u8 temp_v0_2;
    void *temp_v0;

    if (M2C_FIELD(arg0, s16 *, 0x6E) == 0xA) {
        if (!(rand() & 1)) {
            func_801C4CB0(arg0);
            temp_v0 = M2C_FIELD(arg0, void **, 0);
            func_80077B78(arg0, 0x27, M2C_FIELD(temp_v0, u16 *, 0x1C), M2C_FIELD(temp_v0, u8 *, 0x20), &D_801C8478, 1, 0);
        } else {
            func_8004CE44(0x95, arg0 + 8, 0x64, 0);
        }
    }
    func_80075FC0(arg0);
    func_8004ECA8(arg0 + 0x64);
    temp_v0_2 = M2C_FIELD(arg0, u8 *, 0x66) - 1;
    M2C_FIELD(arg0, u8 *, 0x66) = temp_v0_2;
    if (!(temp_v0_2 & 0xFF)) {
        temp_v0_3 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
        if (temp_v0_3 != NULL) {
            temp_v0_3(arg0, M2C_FIELD(arg0, u8 *, 0x41));
        }
    }
}
