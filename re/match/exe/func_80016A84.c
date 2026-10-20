/* exe 0x80016A84  size 72  permuter tier (tools/loop/permute.py): levers etype D_8004A268 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80016A3C(M2C_UNK *);                       /* extern */
extern u8 D_8004A268;

s32 func_80016A84(void) {
    s32 temp_s0;

    temp_s0 = func_80016A3C(&D_8004A268);
    return (temp_s0 & 0xFFFF) | (func_80016A3C(&D_8004A268 + 8) << 0x10);
}
