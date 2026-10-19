/* ovl201 func_8008BD9C - request retirement: the frame loop calls destroy, then frees
 * the slot. */
#include "ac1/ac1.h"
void func_8008BD9C(UiObj *o)
{
    o->kill = 1;
}
