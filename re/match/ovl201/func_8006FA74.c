/* ovl201 0x8006FA74  size 172  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8006FB20(UiObj *);                        /* extern */
extern M2C_UNK D_800B7EBC;

void func_8006FA74(s32 arg0) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(4);
    temp_v0->ctor = func_8006FA74;
    temp_v0->update = func_8006FB20;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->light = 0xA;
    temp_v0->unk14 = 5;
    temp_v0->res = &D_800B7EBC;
    temp_v0->pos.vx = 0xEA;
    temp_v0->pos.vy = 0x62;
    M2C_FIELD(temp_v0, s16 *, 0x7E) = 0xEC;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vz = 0;
    temp_v0->w.list.base = 0x64;
    M2C_FIELD(temp_v0, s32 *, 0x84) = arg0;
}
