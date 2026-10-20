/* ovl201 0x8006F964  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK D_800B8168;

void func_8006F964(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(4);
    temp_v0->ctor = func_8006F964;
    temp_v0->update = ui_obj_nop2;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->light = 4;
    temp_v0->unk14 = 0xA;
    temp_v0->res = &D_800B8168;
    temp_v0->pos.vx = 0xA0;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vy = 0x18;
    temp_v0->pos.vz = 0;
}
