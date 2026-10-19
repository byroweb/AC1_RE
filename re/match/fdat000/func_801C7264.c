/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0032): hand the
 * object to func_800759A4 with the default record when its index at +0x126 is -1,
 * else to func_80075B08 with entry [index] of the 0x34-byte table at 0x80198B08. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x126];
    s16 index;          /* +0x126 */
} Func801C7264Obj;
typedef struct {
    u8 data[0x34];
} Func801C7264Rec;
extern u8 D_80039C78[];
extern Func801C7264Rec D_80198B08[];
extern void func_800759A4(Func801C7264Obj *o, void *arg, void *rec);
extern void func_80075B08(Func801C7264Obj *o, void *arg, void *rec);
void func_801C7264(Func801C7264Obj *o, void *arg)
{
    if (o->index == -1)
        func_800759A4(o, arg, D_80039C78);
    else
        func_80075B08(o, arg, &D_80198B08[o->index]);
}
