#include "ac1_types.h"
typedef struct {
    u8 pad[0x8A];
    s8 index;       /* +0x8A */
} Func800A7A44Sub;
typedef struct {
    u8 pad[0x4C];
    s32 result;     /* +0x4C */
    u8 pad2[0x7C - 0x50];
    Func800A7A44Sub *sub;   /* +0x7C */
} Func800A7A44Obj;
void func_800A7A44(Func800A7A44Obj *o)
{
    o->result = o->sub->index * 24;
}
