/* ovl201 func_8008BF90 - switch update to func_8008BFA0. */
#include "ac1/ac1.h"
extern void func_8008BFA0(UiObj *o);
void func_8008BF90(UiObj *o)
{
    o->update = func_8008BFA0;
}
