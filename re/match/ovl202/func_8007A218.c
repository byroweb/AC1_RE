/* ovl202 0x8007A218  size 284  variant sweep (tools/match/variants.py): ctx-spframe draft + func_80076548(arg0, 0x12), 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 SquareRoot0(s32);                               /* extern */
M2C_UNK func_8004DFF8(void *, s32, M2C_UNK *);      /* extern */
s32 func_8004E06C(s16 *, M2C_UNK *, u32, u32, s32); /* extern */
M2C_UNK func_8004E98C(s32, s32, M2C_UNK, M2C_UNK);  /* extern */
M2C_UNK func_800749A0(void *, void *, M2C_UNK *, M2C_UNK *); /* extern */
M2C_UNK func_80076548(void *, s32);                 /* extern */

void func_8007A218(void *arg0, s32 arg1) {
    union { s8 b[0x18]; s32 w; } sp_frame;
    s32 temp_s0;
    s32 temp_v0;
    s32 temp_v0_2;

    func_80076548(arg0, 0x12);
    if (arg1 != -1) {
        func_8004E98C(arg0 + 0x64, arg1, 1, 0);
    } else {
        M2C_FIELD(arg0, s16 *, 0x70) = 1;
    }
    temp_v0 = (s32) (M2C_FIELD(arg0, u16 *, 0xCC) << 0x10) >> 0x16;
    (*(s16 *)(sp_frame.b + 0x0)) = (s16) temp_v0;
    (*(u16 *)(sp_frame.b + 0x2)) = (u16) ((s32) (M2C_FIELD(arg0, u16 *, 0xCE) << 0x10) >> 0x16);
    temp_v0_2 = (s32) (M2C_FIELD(arg0, u16 *, 0xD0) << 0x10) >> 0x16;
    (*(s16 *)(sp_frame.b + 0x4)) = (s16) temp_v0_2;
    (*(u16 *)(sp_frame.b + 0x2)) += SquareRoot0((temp_v0 * temp_v0) + (temp_v0_2 * temp_v0_2)) >> 1;
    temp_s0 = func_8004E06C(&(*(s16 *)(sp_frame.b + 0x0)), &(*(M2C_UNK *)(sp_frame.b + 0x8)), (u16) M2C_FIELD(arg0, u16 *, 0x28) >> 1, (u16) M2C_FIELD(arg0, u16 *, 0x2A) >> 1, (s32) M2C_FIELD(arg0, u8 *, 0x68));
    func_800749A0(arg0, arg0 + 8, &(*(M2C_UNK *)(sp_frame.b + 0x10)), &(*(M2C_UNK *)(sp_frame.b + 0x8)));
    func_8004DFF8(arg0 + 0x64, temp_s0, &(*(M2C_UNK *)(sp_frame.b + 0x8)));
}
