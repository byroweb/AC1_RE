#include "ac1/ac1.h"
/* ovl201 0x8007BCE8 - build the type-3 label of a part slot: 'L' and 'R'
 * (arms), 'U' and 'D' each have their own image and place. */

extern u8 D_800B83F4[];
extern u8 D_800B8400[];
extern u8 D_800B91AC[];
extern u8 D_800B91B8[];

UiObj *func_8007BCE8(u8 slot)
{
    UiObj *o;

    o = ui_obj_alloc(4);
    o->ctor = func_8007BCE8;
    o->update = ui_obj_nop2;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 11;
    o->unk14 = 1;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    if (slot == 'L') {
        o->res = D_800B83F4;
        o->pos.vx = 8;
        o->pos.vy = 64;
        o->pos.vz = 0;
    } else if (slot == 'R') {
        o->res = D_800B8400;
        o->pos.vx = 120;
        o->pos.vy = 64;
        o->pos.vz = 0;
    } else if (slot == 'U') {
        o->res = D_800B91AC;
        o->pos.vx = 56;
        o->pos.vy = 24;
        o->pos.vz = 0;
    } else if (slot == 'D') {
        o->res = D_800B91B8;
        o->pos.vx = 56;
        o->pos.vy = 112;
        o->pos.vz = 0;
    }
    return o;
}
