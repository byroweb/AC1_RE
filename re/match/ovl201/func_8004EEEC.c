/* ovl201 0x8004EEEC  size 252  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80052134(void *, M2C_UNK);             /* extern */
M2C_UNK func_80070890(M2C_UNK);                     /* extern */
M2C_UNK func_8008729C();                            /* extern */
M2C_UNK func_8008C3EC();                            /* extern */
M2C_UNK func_80096944();                            /* extern */
M2C_UNK func_800B20A4();                            /* extern */
extern u8 D_80039CAA;
extern s8 D_800B91A8;

void func_8004EEEC(void *arg0) {
    s8 temp_v1;

    M2C_FIELD(M2C_FIELD(arg0, void **, 0x9C), s8 *, 1) = 1;
    temp_v1 = M2C_FIELD(arg0, s8 *, 0xAE);
    switch (temp_v1) {
    case 0:
        if (D_80039CAA != 0) {
            D_80039CAA = 0;
            func_80070890(6);
        } else {
            func_80070890(0);
        }
        break;
    case 3:
        func_80052134(arg0, 0);
        break;
    case 4:
        func_80096944();
        break;
    case 1:
        func_8008C3EC();
        D_800B91A8 = 1;
        break;
    case 2:
        func_8008729C();
        break;
    case 5:
        func_800B20A4();
        break;
    }
    M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
}
