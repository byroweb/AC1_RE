/* ovl201 func_8007C68C - constructor: a kind-0 object updated by func_8007C7D8;
 * runs the first step itself and returns the object (the "move v0,s0" before
 * the last store is the return). */
#include "ac1/ac1.h"
extern void func_8007C6FC(UiObj *o);
extern void func_8007C7D8(UiObj *o);
UiObj *func_8007C68C(void)
{
    UiObj *o;
    o = ui_obj_alloc(0);
    o->ctor = (void *)func_8007C68C;
    o->update = func_8007C7D8;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    func_8007C6FC(o);
    o->w.raw[0x80 - 0x7C] = 0;
    return o;
}
