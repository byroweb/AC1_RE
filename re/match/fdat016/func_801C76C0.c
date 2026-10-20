/* fdat016 (16 twins): event hook, func(o, event, ...): event 0x8200 sets the flag byte
 * the object's +0x38 record points at.  Always returns 0. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x38];
    u8 *flag;           /* +0x38 */
} Func801C76C0Obj;
s32 func_801C76C0(Func801C76C0Obj *o, s32 event, s32 arg, ...)
{
    if (event == 0x8200)
        *o->flag = 1;
    return 0;
}
