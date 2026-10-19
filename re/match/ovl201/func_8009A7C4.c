/* ovl201 func_8009A7C4 - switch update to func_8009A7D4. */
#include "ac1/ac1.h"
extern void func_8009A7D4(UiObj *o);
void func_8009A7C4(UiObj *o)
{
    o->update = func_8009A7D4;
}
