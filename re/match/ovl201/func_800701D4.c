/* ovl201 0x800701D4  size 212  permuter tier (tools/loop/permute.py): levers swap temp_v0->w.list.base = a / temp_v0->pos.vy = arg3 + + swap temp_v0->w.list.src = ar / temp_v0->pos.vy = arg3 + + swap temp_v0->pos.vz = 0; / temp_v0->pos.vy = arg3 +, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800702A8(UiObj *);                        /* extern */
extern M2C_UNK D_800B81F8;

void func_800701D4(s16 arg0, u8 *arg1, s16 arg2, s16 arg3) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc((s32) arg0);
    temp_v0->ctor = func_800701D4;
    temp_v0->update = func_800702A8;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->res = &D_800B81F8;
    temp_v0->light = 4;
    temp_v0->unk14 = 2;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vx = (s32) arg2;
    temp_v0->pos.vy = arg3 + (*arg1 * 0x10);
    temp_v0->pos.vz = 0;
    temp_v0->w.list.src = arg1;
    temp_v0->w.list.base = arg3;
}
