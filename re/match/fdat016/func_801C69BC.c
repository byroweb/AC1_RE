/* fdat016 0x801C69BC  size 164  permuter tier (tools/loop/permute.py): levers index D_801D0B94 u8[], from the ctx-fix-frame draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

struct _m2c_stack_func_801C69BC {};                 /* size 0x0 */

extern M2C_UNK D_801A26B8;

extern u8 D_801D0B94[];
void func_801C69BC(void *arg0) {
    M2C_FIELD((*(void **)(D_801D0B94 + ((M2C_FIELD(((s32) arg0), u16 *, 0xFC) << 6)))), s8 *, 0x10) = (s8) ((s32) ((((s32) arg0) - ((s32) &D_801A26B8)) * 0xE9BD37A7) >> 4);
    M2C_FIELD(arg0, s16 *, 0x36) = 1;
    M2C_FIELD(arg0, s16 *, 0x108) = 0x28;
    M2C_FIELD(arg0, s16 *, 0x6A) = 0xFA;
    M2C_FIELD(arg0, s16 *, 0x6C) = 0xFA;
    M2C_FIELD(arg0, s32 *, 0x50) = 0;
    M2C_FIELD(arg0, s32 *, 0x54) = 0;
    M2C_FIELD(arg0, s16 *, 0x10C) = 0;
    M2C_FIELD(arg0, s8 *, 0x42) = 0;
    M2C_FIELD(arg0, u16 *, 0x32) = (u16) (M2C_FIELD(arg0, u16 *, 0x32) | 5);
    M2C_FIELD(arg0, u16 *, 0x10A) = (u16) M2C_FIELD(arg0, u16 *, 0x100);
    M2C_FIELD(arg0, s16 *, 0x10E) = (s16) (M2C_FIELD(arg0, u8 *, 0xFE) * 0x4E2);
}
