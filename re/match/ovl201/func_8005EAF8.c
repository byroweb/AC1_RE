/* ovl201 0x8005EAF8  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK D_800B70B8;

void func_8005EAF8(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(2);
    temp_v0->ctor = func_8005EAF8;
    temp_v0->update = ui_obj_nop2;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->light = 0x15;
    temp_v0->unk14 = 1;
    temp_v0->res = &D_800B70B8;
    temp_v0->pos.vx = 0x20;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vy = 0x78;
    temp_v0->pos.vz = 0;
}
