/* ovl201 func_80099ECC - request retirement: the frame loop calls destroy, then frees
 * the slot. */
#include "ac1/ac1.h"
void func_80099ECC(UiObj *o)
{
    o->kill = 1;
}
