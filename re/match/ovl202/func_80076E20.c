/* ovl202 0x80076E20  size 320  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 SquareRoot0(s32);                               /* extern */
M2C_UNK func_8004DFF8(void *, s32, s16 *);          /* extern */
s32 func_8004E06C(s16 *, s16 *, u32, u32, s32);     /* extern */
s32 func_800749A0(void *, void *, M2C_UNK *, s16 *); /* extern */
M2C_UNK func_80076D20();                            /* extern */

void func_80076E20(void *arg0) {
    union { s8 b[0x18]; s32 w; } sp_frame;
    s32 temp_s0;
    s32 temp_v0;
    s32 temp_v0_2;

    func_80076D20();
    temp_v0 = (s32) (M2C_FIELD(arg0, u16 *, 0xCC) << 0x10) >> 0x16;
    (*(s16 *)(sp_frame.b + 0x0)) = (s16) temp_v0;
    (*(u16 *)(sp_frame.b + 0x2)) = (u16) ((s32) (M2C_FIELD(arg0, u16 *, 0xCE) << 0x10) >> 0x16);
    temp_v0_2 = (s32) (M2C_FIELD(arg0, u16 *, 0xD0) << 0x10) >> 0x16;
    (*(s16 *)(sp_frame.b + 0x4)) = (s16) temp_v0_2;
    (*(u16 *)(sp_frame.b + 0x2)) += SquareRoot0((temp_v0 * temp_v0) + (temp_v0_2 * temp_v0_2)) >> 1;
    temp_s0 = func_8004E06C(&(*(s16 *)(sp_frame.b + 0x0)), &(*(s16 *)(sp_frame.b + 0x8)), (u16) M2C_FIELD(arg0, u16 *, 0x28) >> 1, (u16) M2C_FIELD(arg0, u16 *, 0x2A) >> 1, (s32) M2C_FIELD(arg0, u8 *, 0x68));
    if (func_800749A0(arg0, arg0 + 8, &(*(M2C_UNK *)(sp_frame.b + 0x10)), &(*(s16 *)(sp_frame.b + 0x8))) != 0) {
        M2C_FIELD(arg0, u16 *, 0xCC) = (u16) ((s32) ((*(s16 *)(sp_frame.b + 0x8)) << 6) / temp_s0);
        M2C_FIELD(arg0, u16 *, 0xCE) = (u16) ((s32) ((*(s16 *)(sp_frame.b + 0xA)) << 6) / temp_s0);
        M2C_FIELD(arg0, u16 *, 0xD0) = (u16) ((s32) ((*(s16 *)(sp_frame.b + 0xC)) << 6) / temp_s0);
    }
    func_8004DFF8(arg0 + 0x64, temp_s0, &(*(s16 *)(sp_frame.b + 0x8)));
}
