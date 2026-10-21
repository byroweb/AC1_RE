#include "ac1/ac1.h"
/* ovl201 0x8004EC24 - once the lead object runs func_8004FDEC, switch to
 * func_8004ECAC unless some other item besides the front one is still used. */

extern void func_8004FDEC(UiObj *o);
extern void func_8004ECAC(UiObj *o);

void func_8004EC24(UiObj *o)
{
    s16 i;

    if (o->w.ring.lead->update == func_8004FDEC) {
        for (i = 0; i < 6; i++) {
            if (i != o->w.ring.front) {
                if (o->w.ring.item[i]->used) {
                    return;
                }
                o->update = func_8004ECAC;
            }
        }
    }
}
