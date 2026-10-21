#include "ac1/ac1.h"
/* ovl201 0x8004FFC4 - pick the update step from the flag at +0x7D. */

extern void func_8004FF14(UiObj *o);
extern void func_8004FFF4(UiObj *o);

void func_8004FFC4(UiObj *o)
{
    if (o->w.raw[1]) {
        o->update = func_8004FFF4;
    } else {
        o->update = func_8004FF14;
    }
}
