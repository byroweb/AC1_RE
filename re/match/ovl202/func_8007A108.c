#include "ac1_types.h"
typedef struct {
    u8 pad[0x15C];
    u32 flags;      /* +0x15C */
} Func8007A108Obj;
typedef struct {
    u8 pad[3];
    u8 bit;         /* +0x03 */
} Func8007A108Src;
void func_8007A108(Func8007A108Obj *o, Func8007A108Src *s)
{
    o->flags |= 1 << s->bit;
}
