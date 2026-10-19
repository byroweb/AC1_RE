#include "ac1_types.h"
typedef struct {
    u8 pad[0x4C];
    s32 offset;     /* +0x4C */
    u8 pad2[0x7C - 0x50];
    s8 *src;        /* +0x7C */
} Func80051DCCObj;
void func_80051DCC(Func80051DCCObj *o)
{
    o->offset = *o->src * 24;
}
