/* ovl201 0x8008B574  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8008B5FC(UiObj *);                        /* extern */
extern M2C_UNK D_800B8A8C;

void func_8008B574(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(6);
    temp_v0->ctor = func_8008B574;
    temp_v0->update = func_8008B5FC;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->light = 0xB;
    temp_v0->unk14 = 1;
    temp_v0->res = &D_800B8A8C;
    temp_v0->pos.vx = 0x110;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vy = 0x50;
    temp_v0->pos.vz = 0;
}
