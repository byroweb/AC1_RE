#include "ac1_types.h"
typedef struct {
    u8 pad[0x15C];
    u32 flags;      /* +0x15C */
} Func8007A0C4Obj;
extern u8 D_8004124B;
void func_8007A0C4(Func8007A0C4Obj *o)
{
    o->flags ^= 1 << D_8004124B;
}
