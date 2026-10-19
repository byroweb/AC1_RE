/* ovl201 func_8009A7DC - request retirement: the frame loop calls destroy, then frees
 * the slot. */
#include "ac1/ac1.h"
void func_8009A7DC(UiObj *o)
{
    o->kill = 1;
}
