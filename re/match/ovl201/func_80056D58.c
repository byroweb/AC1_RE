/* ovl201 0x80056D58  size 412  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005DED8(void *, M2C_UNK *, M2C_UNK *, M2C_UNK); /* extern */
s32 func_800684A0(u8);                              /* extern */
s32 func_800B498C(u8, s32);                         /* extern */
extern M2C_UNK D_8004B1E8;
extern M2C_UNK D_8004B258;
extern M2C_UNK D_8004B26C;
extern s16 D_801A2318;
extern M2C_UNK func_800537E0;
extern M2C_UNK func_80054258;
extern M2C_UNK func_800549D0;
extern M2C_UNK func_8005552C;

void func_80056D58(void *arg0) {
    M2C_UNK *var_a1;
    M2C_UNK *var_a2;
    u8 temp_v1;
    void *temp_a0;

    temp_a0 = arg0 + 0x7C;
    M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0xA4) * 4) + temp_a0), s32 *, 0x1C) = func_800684A0(M2C_FIELD(arg0, u8 *, 0xA4));
    temp_v1 = M2C_FIELD(arg0, u8 *, 0xA4);
    if (M2C_FIELD(((temp_v1 * 4) + temp_a0), s32 *, 0x1C) != 0) {
        func_800506EC(0xA);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0xC, 0x68, 0x128, 0x20, 9);
        func_8005DED8(arg0, &func_800549D0, &D_8004B1E8, 0xC);
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
    } else {
        if (func_800B498C(temp_v1, M2C_FIELD(arg0, u8 *, 0xA8) + (M2C_FIELD(arg0, u8 *, 0xA9) * 4)) & 0xFF) {
            if (M2C_FIELD(arg0, s32 *, 0x80) == 0) {
                var_a1 = &func_800537E0;
                var_a2 = &D_8004B258;
            } else {
                var_a1 = &func_8005552C;
                var_a2 = &D_8004B258;
            }
        } else {
            var_a1 = &func_80054258;
            var_a2 = &D_8004B26C;
        }
        func_8005DED8(arg0, var_a1, var_a2, 0xC);
        func_800506EC(0xA);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0xC, 0x68, 0x128, 0x20, 9);
    }
    M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
}
