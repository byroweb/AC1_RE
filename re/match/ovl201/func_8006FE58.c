#include "ac1/ac1.h"

extern u8 D_800B81BC[];
extern u8 D_800B81C8[];

UiObj *func_8006FE58(s16 corner)
{
    UiObj *o;

    o = ui_obj_alloc(4);
    o->ctor = func_8006FE58;
    o->update = ui_obj_nop2;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 4;
    o->unk14 = 9;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    switch (corner) {
    case 0:
        o->res = D_800B81BC;
        o->pos.vx = 171;
        o->pos.vy = 28;
        o->pos.vz = 0;
        break;
    case 1:
        o->res = D_800B81C8;
        o->pos.vx = 300;
        o->pos.vy = 35;
        o->pos.vz = 0;
        break;
    case 2:
        o->res = D_800B81BC;
        o->pos.vx = 171;
        o->pos.vy = 164;
        o->pos.vz = 0;
        break;
    case 3:
        o->res = D_800B81C8;
        o->pos.vx = 164;
        o->pos.vy = 35;
        o->pos.vz = 0;
        break;
    }
    return o;
}
