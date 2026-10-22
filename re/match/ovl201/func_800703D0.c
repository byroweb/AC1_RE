#include "ac1/ac1.h"
void func_8007046C(UiObj *o);

void func_800703D0(UiObj *o)
{
    UiObj *ref = *(UiObj **)&o->w.raw[0x98 - 0x7C];
    UiObj *n = ui_obj_alloc(4);
    n->ctor = func_800703D0;
    n->update = func_8007046C;
    n->destroy = ui_obj_nop;
    n->light = 4;
    n->visible = 0;
    n->unk14 = 3;
    n->has_xform = 0;
    n->rot.vx = 0;
    n->rot.vy = 0;
    n->rot.vz = 0;
    n->pos.vx = ref->pos.vx;
    n->pos.vy = ref->pos.vy;
    n->pos.vz = 0;
    n->w.list.src = o;
}
