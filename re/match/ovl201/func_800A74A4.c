/* ovl201 func_800A74A4 - switch update to func_800A74B4. */
#include "ac1/ac1.h"
extern void func_800A74B4(UiObj *o);
void func_800A74A4(UiObj *o)
{
    o->update = func_800A74B4;
}
