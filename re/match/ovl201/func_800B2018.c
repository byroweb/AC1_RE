/* ovl201 func_800B2018 - list row: y = (byte at src) * 24. */
#include "ac1/ac1.h"
void func_800B2018(UiObj *o)
{
    o->pos.vy = *(u8 *)o->w.list.src * 24;
}
