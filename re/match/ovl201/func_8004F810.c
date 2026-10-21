#include "ac1/ac1.h"
/* ovl201 0x8004F810 - copy src's resource into a new type-3 object drawn
 * kind * 16 down-right of src; the kind (1..3) picks light and layer. */

extern void func_8004FD4C(UiObj *o);

UiObj *func_8004F810(UiObj *src, s16 kind)
{
    UiObj *o;

    o = ui_obj_alloc(2);
    o->ctor = func_8004F810;
    o->update = func_8004FD4C;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->res = src->res;
    switch (kind) {
    case 1:
        o->light = 1;
        o->unk14 = 3;
        break;
    case 2:
        o->light = 2;
        o->unk14 = 4;
        break;
    case 3:
        o->light = 3;
        o->unk14 = 5;
        break;
    }
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->pos.vx = src->pos.vx + (kind << 4);
    o->pos.vy = src->pos.vy + (kind << 4);
    o->pos.vz = 0;
    return o;
}
