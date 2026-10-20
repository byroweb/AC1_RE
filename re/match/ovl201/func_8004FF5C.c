/* ovl201 0x8004FF5C  size 104  variant sweep (tools/match/variants.py): args lever: args func_80052134 +arg0 proto, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8004FFF4(UiObj *o);                       /* extern */
M2C_UNK func_80052134(void *, M2C_UNK *);                   /* extern */
extern M2C_UNK func_8004FFC4;

void func_8004FF5C(void *arg0) {
    u8 temp_v1;
    void (*var_v0)(UiObj *);

    temp_v1 = M2C_FIELD(arg0, u8 *, 0x7C);
    if (temp_v1 == 0) {
        func_80052134(arg0, &func_8004FFC4);
        var_v0 = ui_obj_nop2;
        goto block_4;
    }
    if (temp_v1 == 1) {
        var_v0 = func_8004FFF4;
block_4:
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = var_v0;
    }
}
