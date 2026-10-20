/* ovl201 0x8005CB94  size 180  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8005CB94(void *arg0, s16 arg1, s16 arg2, s16 arg3) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc((s32) arg3);
    temp_v0->ctor = func_8005CB94;
    temp_v0->update = ui_obj_nop2;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 6;
    temp_v0->light = 0x15;
    temp_v0->unk14 = 1;
    temp_v0->res = arg0;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vx = (s32) arg1;
    temp_v0->pos.vy = (s32) arg2;
    temp_v0->pos.vz = 0;
}
