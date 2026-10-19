/* ovl201 func_80095F4C - request retirement: the frame loop calls destroy, then frees
 * the slot. */
#include "ac1/ac1.h"
void func_80095F4C(UiObj *o)
{
    o->kill = 1;
}
