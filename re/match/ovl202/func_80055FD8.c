/* ovl202 0x80055FD8  size 284  zero-agent sweep (tools/loop/sweep.py): m2c ctx-knr draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80055E84();            /* extern */

void func_80055FD8(void *arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    s32 temp_a2;

    temp_a2 = arg2 >> 1;
    func_80055E84(M2C_FIELD(arg0, s32 *, 0x5C) + temp_a2, arg0 + 0x60, temp_a2);
    func_80055E84(M2C_FIELD(arg0, s32 *, 0x5C) + (arg3 >> 1), arg0 + 0x68);
    func_80055E84(M2C_FIELD(arg0, s32 *, 0x5C) + (arg4 >> 1), arg0 + 0x70);
    func_80055E84(M2C_FIELD(arg0, s32 *, 0x5C) + (arg5 >> 1), arg0 + 0x78);
    M2C_FIELD(arg1, s16 *, 0) = (s16) ((s32) (M2C_FIELD(arg0, s16 *, 0x60) + M2C_FIELD(arg0, s16 *, 0x68) + M2C_FIELD(arg0, s16 *, 0x70) + M2C_FIELD(arg0, s16 *, 0x78)) >> 2);
    M2C_FIELD(arg1, s16 *, 2) = (s16) ((s32) (M2C_FIELD(arg0, s16 *, 0x62) + M2C_FIELD(arg0, s16 *, 0x6A) + M2C_FIELD(arg0, s16 *, 0x72) + M2C_FIELD(arg0, s16 *, 0x7A)) >> 2);
    M2C_FIELD(arg1, s16 *, 4) = (s16) ((s32) (M2C_FIELD(arg0, s16 *, 0x64) + M2C_FIELD(arg0, s16 *, 0x6C) + M2C_FIELD(arg0, s16 *, 0x74) + M2C_FIELD(arg0, s16 *, 0x7C)) >> 2);
}
