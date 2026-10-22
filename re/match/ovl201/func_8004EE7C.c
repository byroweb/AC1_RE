#include "ac1/ac1.h"
/* ovl201 0x8004EE7C - once func_800517C8 says 0, raise flag 0x40, drop the
 * held confirm button and go to func_8004EEEC. */

extern u32 D_801A2550;
extern u32 D_801A2568;
extern s16 func_800517C8(void);
extern void func_8004EEEC(UiObj *o);

void func_8004EE7C(UiObj *o)
{
    u32 *flags;

    if (func_800517C8() == 0) {
        flags = &D_801A2550;
        *flags |= 0x40;
        D_801A2568 &= ~0x40;
        o->update = func_8004EEEC;
    }
}
