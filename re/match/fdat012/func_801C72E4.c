/* fdat012 0x801C72E4  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_80073674(M2C_UNK);                       /* extern */
s32 func_801C729C(u8, u16);                         /* extern */
extern M2C_UNK func_801C715C;

void func_801C72E4(void *arg0) {
    void *temp_v0;

    if ((func_801C729C(M2C_FIELD(arg0, u8 *, 0x22), M2C_FIELD(arg0, u16 *, 0x24)) != 0) && (func_801C729C(M2C_FIELD(arg0, u8 *, 0x2A), M2C_FIELD(arg0, u16 *, 0x2C)) != 0)) {
        temp_v0 = func_80073674(4);
        M2C_FIELD(arg0, void **, 0x14) = temp_v0;
        M2C_FIELD(temp_v0, M2C_UNK **, 4) = &func_801C715C;
        return;
    }
    M2C_FIELD(arg0, s16 *, 6) = -1;
}
