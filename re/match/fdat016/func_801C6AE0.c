#include "ac1_types.h"
typedef struct { u8 pad[0x36]; u16 state; u8 pad2[0x108 - 0x38]; void *handler; } Obj;
extern void func_801CDC7C();
extern void func_801CDBD4(Obj *o);
void func_801C6AE0(Obj *o)
{
    o->state = 0x102;
    o->handler = func_801CDC7C;
    func_801CDBD4(o);
}
