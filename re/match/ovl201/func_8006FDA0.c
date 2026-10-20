/* ovl201 0x8006FDA0  size 152  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8006FE38(UiObj *);                        /* extern */
extern M2C_UNK D_800B81B0;

void func_8006FDA0(void *arg0) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(4);
    temp_v0->ctor = func_8006FDA0;
    temp_v0->update = func_8006FE38;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->unk14 = 6;
    temp_v0->res = &D_800B81B0;
    temp_v0->pos.vx = 0xA8;
    temp_v0->light = 0;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vy = 0xBC;
    temp_v0->pos.vz = 0;
    temp_v0->w.list.src = arg0;
}
