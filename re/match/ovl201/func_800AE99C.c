/* ovl201 0x800AE99C  size 164  permuter tier (tools/loop/permute.py): levers swap temp_v0->w.list.src = ar / temp_v0->pos.vy = *arg0  + swap temp_v0->pos.vz = 0; / temp_v0->pos.vy = *arg0 , from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800AEA40(UiObj *);                        /* extern */
extern M2C_UNK D_800BDDE8;

void func_800AE99C(u8 *arg0) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(4);
    temp_v0->ctor = func_800AE99C;
    temp_v0->update = func_800AEA40;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->light = 0x15;
    temp_v0->unk14 = 1;
    temp_v0->res = &D_800BDDE8;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vx = 0;
    temp_v0->pos.vy = *arg0 * 0x18;
    temp_v0->pos.vz = 0;
    temp_v0->w.list.src = arg0;
}
