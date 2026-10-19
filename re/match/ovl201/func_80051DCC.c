/* ovl201 func_80051DCC - list row: y = (signed byte at src) * 24. */
#include "ac1/ac1.h"
void func_80051DCC(UiObj *o)
{
    o->pos.vy = *(s8 *)o->w.list.src * 24;
}
