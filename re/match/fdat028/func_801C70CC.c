/* fdat028 0x801C70CC  size 232  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_80086F20(M2C_UNK, s16 *, M2C_UNK, M2C_UNK, s32, M2C_UNK *, M2C_UNK *, M2C_UNK *, M2C_UNK *); /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK D_801C74DC;
extern M2C_UNK D_801C74E4;
extern M2C_UNK D_801C74EC;
extern M2C_UNK D_801C74F0;
extern M2C_UNK func_801C6F28;

void func_801C70CC(void *arg0) {
    union { s8 b[0x8]; s32 w; } sp_frame;
    void *temp_v0;

    if (M2C_FIELD(arg0, s16 *, 6) != 0) {
        (*(s16 *)(sp_frame.b + 0x0)) = (M2C_FIELD(arg0, u16 *, 0) + (rand() & 0xFF)) - 0x80;
        (*(s16 *)(sp_frame.b + 0x2)) = (M2C_FIELD(arg0, u16 *, 2) + (rand() & 0xFF)) - 0x80;
        (*(s16 *)(sp_frame.b + 0x4)) = (M2C_FIELD(arg0, u16 *, 4) + (rand() & 0xFF)) - 0x80;
        temp_v0 = func_80086F20(0, &(*(s16 *)(sp_frame.b + 0x0)), 0x29, 0x50, (rand() & 7) + 0xA, &D_801C74DC, &D_801C74E4, &D_801C74EC, &D_801C74F0);
        M2C_FIELD(temp_v0, s16 *, 0x12) = 0x14;
        M2C_FIELD(temp_v0, M2C_UNK **, 0x14) = &func_801C6F28;
    }
}
