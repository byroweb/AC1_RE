/* exe 0x800140C4  size 472  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800140C4(void *arg0, void *arg1) {
    M2C_FIELD(arg1, s16 *, 0) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 8) * M2C_FIELD(arg0, s16 *, 0x10)) - (M2C_FIELD(arg0, s16 *, 0xA) * M2C_FIELD(arg0, s16 *, 0xE))) >> 0xC);
    M2C_FIELD(arg1, s16 *, 6) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 0xA) * M2C_FIELD(arg0, s16 *, 0xC)) - (M2C_FIELD(arg0, s16 *, 6) * M2C_FIELD(arg0, s16 *, 0x10))) >> 0xC);
    M2C_FIELD(arg1, s16 *, 0xC) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 6) * M2C_FIELD(arg0, s16 *, 0xE)) - (M2C_FIELD(arg0, s16 *, 8) * M2C_FIELD(arg0, s16 *, 0xC))) >> 0xC);
    M2C_FIELD(arg1, s16 *, 2) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 4) * M2C_FIELD(arg0, s16 *, 0xE)) - (M2C_FIELD(arg0, s16 *, 2) * M2C_FIELD(arg0, s16 *, 0x10))) >> 0xC);
    M2C_FIELD(arg1, s16 *, 8) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 0) * M2C_FIELD(arg0, s16 *, 0x10)) - (M2C_FIELD(arg0, s16 *, 4) * M2C_FIELD(arg0, s16 *, 0xC))) >> 0xC);
    M2C_FIELD(arg1, s16 *, 0xE) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 2) * M2C_FIELD(arg0, s16 *, 0xC)) - (M2C_FIELD(arg0, s16 *, 0) * M2C_FIELD(arg0, s16 *, 0xE))) >> 0xC);
    M2C_FIELD(arg1, s16 *, 4) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 2) * M2C_FIELD(arg0, s16 *, 0xA)) - (M2C_FIELD(arg0, s16 *, 4) * M2C_FIELD(arg0, s16 *, 8))) >> 0xC);
    M2C_FIELD(arg1, s16 *, 0xA) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 4) * M2C_FIELD(arg0, s16 *, 6)) - (M2C_FIELD(arg0, s16 *, 0) * M2C_FIELD(arg0, s16 *, 0xA))) >> 0xC);
    M2C_FIELD(arg1, s16 *, 0x10) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 0) * M2C_FIELD(arg0, s16 *, 8)) - (M2C_FIELD(arg0, s16 *, 2) * M2C_FIELD(arg0, s16 *, 6))) >> 0xC);
}
