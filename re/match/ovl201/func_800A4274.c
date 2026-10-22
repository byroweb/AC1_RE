#include "ac1/ac1.h"
/* ovl201 0x800A4274 - credits: build the left ('L') or right panel sprite */
extern u8 D_800BDBC0[];
extern u8 D_800BDBCC[];

UiObj *func_800A4274(u8 side)
{
    UiObj *o;

    o = ui_obj_alloc(2);
    o->ctor = func_800A4274;
    o->update = ui_obj_nop2;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 0xB;
    o->unk14 = 2;
    if (side == 'L') {
        o->res = D_800BDBC0;
    } else {
        o->res = D_800BDBCC;
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
        o->pos.vx = 0x100;
        o->pos.vy = 0;
        o->pos.vz = 0;
    }
    return o;
}
