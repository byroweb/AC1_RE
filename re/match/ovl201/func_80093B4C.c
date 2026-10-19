/* ovl201 func_80093B4C - request retirement: the frame loop calls destroy, then frees
 * the slot. */
#include "ac1/ac1.h"
void func_80093B4C(UiObj *o)
{
    o->kill = 1;
}
