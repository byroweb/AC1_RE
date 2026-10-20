/* ovl201 0x8004F924  size 180  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8004F9D8(UiObj *);                        /* extern */
M2C_UNK func_80050AF4(M2C_UNK, M2C_UNK);            /* extern */
M2C_UNK func_80051450(M2C_UNK, M2C_UNK);            /* extern */
M2C_UNK func_8005D39C(M2C_UNK);                     /* extern */

UiObj *func_8004F924(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(2);
    temp_v0->ctor = func_8004F924;
    temp_v0->update = func_8004F9D8;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 8;
    temp_v0->light = 0;
    temp_v0->unk14 = 2;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vx = 0;
    temp_v0->pos.vy = 0;
    temp_v0->pos.vz = 0;
    M2C_FIELD(temp_v0, s16 *, 0x7C) = 0;
    func_8005D39C(2);
    func_80051450(4, 1);
    func_80051450(4, 4);
    func_80050AF4(0xF, 1);
    return temp_v0;
}
