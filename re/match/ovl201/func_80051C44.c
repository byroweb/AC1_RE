#include "ac1/ac1.h"
/* ovl201 0x80051C44 - spawn the left ('L') or right scroll arrow */
extern u8 D_800B6F40[];
extern u8 D_800B6F4C[];
void func_80051D14(UiObj *o);

UiObj *func_80051C44(void *src, u8 side)
{
    UiObj *o;

    o = ui_obj_alloc(8);
    o->ctor = func_80051C44;
    o->update = func_80051D14;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 21;
    o->unk14 = 1;
    if (side == 'L') {
        o->res = D_800B6F40;
        o->pos.vx = 0;
        o->pos.vy = 0;
        o->pos.vz = 0;
    } else {
        o->res = D_800B6F4C;
        o->pos.vx = 248;
        o->pos.vy = 0;
        o->pos.vz = 0;
    }
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->w.list.src = src;
    return o;
}
