#include "ac1_types.h"
typedef struct {
    u8 pad[0x4C];
    s32 result;     /* +0x4C */
    u8 pad2[0x7C - 0x50];
    u8 *ptr;        /* +0x7C */
    s16 base;       /* +0x80 */
} Func800702A8Obj;
void func_800702A8(Func800702A8Obj *o)
{
    o->result = o->base + (*o->ptr << 4);
}
