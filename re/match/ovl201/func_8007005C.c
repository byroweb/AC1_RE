/* ovl201 0x8007005C  size 124  variant sweep (tools/match/variants.py): swap lever (store order), from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK D_800B81D4;

void func_8007005C(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0xA);
    temp_v0->ctor = func_8007005C;
    temp_v0->update = ui_obj_nop2;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->render_type = 3;
    temp_v0->res = &D_800B81D4;
    temp_v0->light = 4;
    temp_v0->unk14 = 3;
    temp_v0->has_xform = 0;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
    temp_v0->pos.vx = 0;
    temp_v0->pos.vy = 0;
    temp_v0->pos.vz = 0;
}
