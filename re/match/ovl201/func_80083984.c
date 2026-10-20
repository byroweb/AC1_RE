/* ovl201 0x80083984  size 164  permuter tier (tools/loop/permute.py): levers swap temp_v0->pos.vz = 0; / temp_v0->pos.vx = (M2C_F + swap temp_v0->pos.vy = 0x1C; / temp_v0->pos.vx = (M2C_F, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80083A28(UiObj *);                        /* extern */
extern M2C_UNK D_800B8550;

void func_80083984(void *arg0) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(6);
    temp_v0->ctor = func_80083984;
    temp_v0->update = func_80083A28;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->light = 0xB;
    temp_v0->unk14 = 1;
    temp_v0->res = &D_800B8550;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->w.list.src = arg0;
    temp_v0->pos.vx = (M2C_FIELD(arg0, u8 *, 0x96) * 0x10) + 8;
    temp_v0->pos.vy = 0x1C;
    temp_v0->pos.vz = 0;
}
