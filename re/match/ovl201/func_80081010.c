/* ovl201 0x80081010  size 176  permuter tier (tools/loop/permute.py): levers swap temp_v0->pos.vz = 0; / temp_v0->pos.vy = ((M2C_ + swap temp_v0->pos.vx = 0; / temp_v0->pos.vy = ((M2C_, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800810C0(UiObj *);                        /* extern */
extern M2C_UNK D_800B846C;

void func_80081010(void *arg0) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(4);
    temp_v0->ctor = func_80081010;
    temp_v0->update = func_800810C0;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->light = 0x15;
    temp_v0->unk14 = 1;
    temp_v0->res = &D_800B846C;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->w.list.src = arg0;
    temp_v0->pos.vy = ((M2C_FIELD(arg0, u8 *, 0x8C) - M2C_FIELD(arg0, u8 *, 0x8D)) & 0xFF) * 0x18;
    temp_v0->pos.vx = 0;
    temp_v0->pos.vz = 0;
}
