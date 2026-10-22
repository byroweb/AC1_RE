/* ovl201 0x80087EB8  size 160  permuter tier (tools/loop/permute.py): levers imm 4->2 + imm 2->1, from the file draft, 2026-10-22 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK func_8005D15C(s8 *, M2C_UNK, s32);          /* extern */
M2C_UNK func_80087C94(s32, s8 *);                   /* extern */
extern M2C_UNK D_801F4114;

void func_80087EB8(s32 arg0, s32 arg1) {
    union { s8 b[0x8]; s32 w; } sp_frame;
    s32 temp_s0;
    s32 temp_v1;

    temp_s0 = arg0 & 0xFF;
    temp_v1 = temp_s0 * 1;
    func_80087C94(*(temp_v1 + &D_801F4114) + M2C_FIELD((&D_801F4114 + temp_v1), s32 *, 0x100), &(*(s8 *)(sp_frame.b + 0x0)));
    if (temp_s0 == 0x1C) {
        (*(s8 *)(sp_frame.b + 0x0)) = 0x2D;
        (*(s8 *)(sp_frame.b + 0x1)) = 0x2D;
        (*(s8 *)(sp_frame.b + 0x2)) = 0x2D;
        (*(s8 *)(sp_frame.b + 0x3)) = 0x2D;
        (*(s8 *)(sp_frame.b + 0x4)) = 0x2D;
        (*(s8 *)(sp_frame.b + 0x5)) = 0x2D;
        (*(s8 *)(sp_frame.b + 0x6)) = 0;
    }
    func_8005D15C(&(*(s8 *)(sp_frame.b + 0x0)), 0x3C0, ((arg1 & 0xFF) * 0x10) + 0xB0);
    DrawSync(0);
}
