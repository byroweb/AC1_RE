/* fdat000 (mission library, entry 0 @ 0x801C4B40; 62 twins): react toward the unit's
 * current path node (func_80075B08 with the node position), or without a node (-1)
 * through func_80075A90, which also gets the third argument. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x126];
    s16 node;           /* +0x126 */
} Func801C72C4Obj;
extern void func_80075A90(Func801C72C4Obj *o, s32 kind, void *arg);
extern void func_80075B08(Func801C72C4Obj *o, s32 kind, AcPos *at);
extern struct { AcPos pos; u8 pad[0x34 - 8]; } D_80198B08[];
void func_801C72C4(Func801C72C4Obj *o, s32 kind, void *arg)
{
    if (o->node == -1)
        func_80075A90(o, kind, arg);
    else
        func_80075B08(o, kind, &D_80198B08[o->node].pos);
}
