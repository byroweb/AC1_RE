/* ovl201 func_800702A8 - list row: y = base + (byte at src) * 16. */
#include "ac1/ac1.h"
void func_800702A8(UiObj *o)
{
    o->pos.vy = o->w.list.base + (*(u8 *)o->w.list.src << 4);
}
