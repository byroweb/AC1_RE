/* ovl201 0x800A7244  size 200  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

UiObj *func_800500EC(M2C_UNK *, UiObj *);           /* extern */
s32 func_800517C8();                                /* extern */
M2C_UNK func_8005CB94(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
void *func_80072838();                              /* extern */
extern M2C_UNK D_8004D358;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_80072C80;
extern M2C_UNK func_800A730C;

void func_800A7244(void *arg0) {
    UiObj *temp_v0;
    UiObj *var_v0;

    temp_v0 = obj_find_by_handle(&func_8005BA2C);
    if ((temp_v0 == &g_ui_none) && ((func_800517C8() << 0x10) == 0)) {
        M2C_FIELD(func_80072838(), void (**)(UiObj *), 0x74) = ui_obj_nop2;
        var_v0 = obj_find_by_handle(&func_80072C80);
        do {
            var_v0->update = ui_obj_nop2;
            var_v0 = func_800500EC(&func_80072C80, var_v0);
        } while (var_v0 != temp_v0);
        func_8005CB94(&D_8004D358, 8, 8, 0xC);
        M2C_FIELD(arg0, s32 *, 0x84) = 0;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A730C;
    }
}
