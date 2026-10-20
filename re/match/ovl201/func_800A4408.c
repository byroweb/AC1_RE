/* ovl201 0x800A4408  size 132  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK D_800BDBF0;

void func_800A4408(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(2);
    temp_v0->ctor = func_800A4408;
    temp_v0->update = ui_obj_nop2;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->unk14 = 2;
    temp_v0->res = &D_800BDBF0;
    temp_v0->pos.vx = 0x5C;
    temp_v0->light = 0;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vy = 0xB0;
    temp_v0->pos.vz = 0;
}
