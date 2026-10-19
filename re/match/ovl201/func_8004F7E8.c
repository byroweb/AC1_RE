/* ovl201 func_8004F7E8 - turn the object by (64, 32, 16) per call. */
#include "ac1/ac1.h"
void func_8004F7E8(UiObj *o)
{
    o->rot.vx += 64;
    o->rot.vy += 32;
    o->rot.vz += 16;
}
