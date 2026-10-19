/* ovl202 func_8007A0C4 - toggle AI flag bit D_8004124B of an entity. */
#include "ac1/ac1.h"
extern u8 D_8004124B;
void func_8007A0C4(AcEntity *e)
{
    e->flags ^= 1 << D_8004124B;
}
