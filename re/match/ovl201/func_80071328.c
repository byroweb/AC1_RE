/* ovl201 0x80071328  size 308  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005DED8(void *, void (*)(UiObj *), M2C_UNK *, M2C_UNK); /* extern */
void func_80070A44(UiObj *o);                       /* extern */
void func_8007145C(UiObj *o);                       /* extern */
s32 func_800714FC(M2C_UNK *, M2C_UNK *);            /* extern */
extern M2C_UNK D_80031BD4;
extern M2C_UNK D_80048510;
extern M2C_UNK D_8004BD20;
extern M2C_UNK func_8005CC48;

void func_80071328(void *arg0) {
    u8 temp_v1;
    void (*var_v0)(UiObj *);

    temp_v1 = M2C_FIELD(arg0, u8 *, 0x89);
    if (temp_v1 == 0) {
        if (func_800714FC(&D_80048510, &D_80031BD4) & 0xFF) {
            M2C_FIELD(M2C_FIELD(arg0, void **, 0x84), s8 *, 1) = 1;
            M2C_FIELD(M2C_FIELD(arg0, void **, 0x80), s8 *, 1) = 1;
            func_800506EC(4);
            func_800506EC(6);
            obj_find_by_id(3)->kill = 1;
            obj_find_by_id(5)->kill = 1;
            obj_find_by_handle(&func_8005CC48)->kill = 1;
            func_8005C2D0(0x20, 0x28, 0x80, 0xA8, 3);
            func_8005C2D0(0xB0, 0x18, 0x80, 0x90, 5);
            var_v0 = func_8007145C;
        } else {
            func_8005DED8(arg0, func_80070A44, &D_8004BD20, 0xC);
            var_v0 = ui_obj_nop2;
        }
        goto block_6;
    }
    if (temp_v1 == 1) {
        var_v0 = func_80070A44;
block_6:
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = var_v0;
    }
}
