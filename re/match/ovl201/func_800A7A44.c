/* ovl201 func_800A7A44 - list row: y = (selected index of the object at src) * 24. */
#include "ac1/ac1.h"
void func_800A7A44(UiObj *o)
{
    o->pos.vy = ((UiObj *)o->w.list.src)->w.sel.index * 24;
}
