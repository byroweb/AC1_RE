/* ovl201 func_800551A0 - switch update to func_800551B0. */
#include "ac1/ac1.h"
extern void func_800551B0(UiObj *o);
void func_800551A0(UiObj *o)
{
    o->update = func_800551B0;
}
