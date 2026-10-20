/* ovl201 0x80072788  size 148  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8007281C(UiObj *);                        /* extern */
extern M2C_UNK D_800B83AC;

void func_80072788(u8 arg0) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(4);
    temp_v0->ctor = func_80072788;
    temp_v0->update = func_8007281C;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->light = 0x15;
    temp_v0->unk14 = 1;
    temp_v0->res = &D_800B83AC;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vx = 0;
    temp_v0->pos.vy = 0;
    temp_v0->pos.vz = 0;
    temp_v0->w.raw[0] = arg0;
}
