/* ovl201 0x8004E0B4  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8004E2BC(UiObj *o);                       /* extern */
M2C_UNK func_8005E23C(void *, void (*)(UiObj *), M2C_UNK *, M2C_UNK); /* extern */
s32 func_80096550();                                /* extern */
extern u8 D_80039CB0;
extern M2C_UNK D_8004ADC4;

void func_8004E0B4(void *arg0) {
    void (*var_v0)(UiObj *);

    if ((func_80096550() & 0xFF) && (D_80039CB0 == 0)) {
        func_8005E23C(arg0, func_8004E2BC, &D_8004ADC4, 6);
        D_80039CB0 = 1;
        var_v0 = ui_obj_nop2;
    } else {
        var_v0 = func_8004E2BC;
    }
    M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = var_v0;
}
