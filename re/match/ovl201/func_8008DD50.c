/* ovl201 func_8008DD50 - constructor: a kind-4 object with no transform, no
 * parent, hidden, updated by func_8008DDA0. */
#include "ac1/ac1.h"
extern void func_8008DDA0(UiObj *o);
void func_8008DD50(void)
{
    UiObj *o = ui_obj_alloc(4);
    o->ctor = (void *)func_8008DD50;
    o->update = func_8008DDA0;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
}
