#include "ac1_types.h"
typedef struct {
    u8 pad[0x4C];
    s32 offset;     /* +0x4C */
    u8 pad2[0x7C - 0x50];
    u8 *src;        /* +0x7C */
} Func800B2018Obj;
void func_800B2018(Func800B2018Obj *o)
{
    o->offset = *o->src * 24;
}
