#include "ac1/ac1.h"
/* ovl201 0x8004F024 - give the six ring items their angles from the
 * D_800B6DEC table, starting at the front item and wrapping. */

extern u16 D_800B6DEC[];

void func_8004F024(UiObj *o)
{
    s8 k = o->w.ring.front;
    u8 i;

    for (i = 0; i < 6; i++) {
        o->w.ring.item[k++]->rot.vy = D_800B6DEC[i];
        if (k == 6) {
            k = 0;
        }
    }
}
