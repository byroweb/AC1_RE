/* ovl201 func_800A4340 - constructor: a kind-2 object drawn with render type 3
 * whose resource and X position depend on whether the argument is 'L'.  Both
 * arms of the last if end with the same two stores (the compiler cross-jumps
 * them after scheduling, which is why retail has the stores before the return
 * move). */
#include "ac1/ac1.h"
extern u8 D_800BDBD8[];
extern u8 D_800BDBE4[];
UiObj *func_800A4340(u8 side)
{
    UiObj *o;
    o = ui_obj_alloc(2);
    o->ctor = (void *)func_800A4340;
    o->update = ui_obj_nop2;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 4;
    o->unk14 = 3;
    if (side == 'L') {
        o->res = D_800BDBD8;
    } else {
        o->res = D_800BDBE4;
    }
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    if (side == 'L') {
        o->pos.vx = 0;
        o->pos.vy = 0;
        o->pos.vz = 0;
    } else {
        o->pos.vx = 256;
        o->pos.vy = 0;
        o->pos.vz = 0;
    }
    return o;
}
