/* ovl202 0x8008AE44  size 232  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8008AD80(M2C_UNK, M2C_UNK, s32);       /* extern */

void func_8008AE44(void *arg0) {
    if (M2C_FIELD(arg0, s32 *, 0x1C) != 0) {
        func_8008AD80(0xCA, 0x38, M2C_FIELD(arg0, s32 *, 0));
    }
    if (M2C_FIELD(arg0, s32 *, 0x20) != 0) {
        func_8008AD80(0xCA, 0x48, M2C_FIELD(arg0, s32 *, 4));
    }
    if (M2C_FIELD(arg0, s32 *, 0x24) != 0) {
        func_8008AD80(0xCA, 0x60, M2C_FIELD(arg0, s32 *, 8));
    }
    if (M2C_FIELD(arg0, s32 *, 0x28) != 0) {
        func_8008AD80(0xCA, 0x70, M2C_FIELD(arg0, s32 *, 0xC));
    }
    if (M2C_FIELD(arg0, s32 *, 0x2C) != 0) {
        func_8008AD80(0xCA, 0x80, M2C_FIELD(arg0, s32 *, 0x10));
    }
    if (M2C_FIELD(arg0, s32 *, 0x30) != 0) {
        func_8008AD80(0xCA, 0x98, M2C_FIELD(arg0, s32 *, 0x14));
    }
    if (M2C_FIELD(arg0, s32 *, 0x34) != 0) {
        func_8008AD80(0xCA, 0xC0, M2C_FIELD(arg0, s32 *, 0x18));
    }
}
