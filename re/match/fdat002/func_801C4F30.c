/* fdat002 (16 twins): spin step (func_801C4EB8), clamp the spin to at most 24, turn the yaw by it. */
#include "ac1/ac1.h"
typedef struct Func801C4F30Obj Func801C4F30Obj;
struct Func801C4F30Obj {
    u8 pad0[0x12];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x41 - 0x14];
    u8 b41;             /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn)(Func801C4F30Obj *, s32); /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[2];        /* +0x64 */
    u8 count;           /* +0x66 */
    u8 pad67[0xCC - 0x67];
    s16 spin;           /* +0xCC */
    s16 spin_acc;       /* +0xCE */
};
extern void func_801C4EB8(Func801C4F30Obj *o);
void func_801C4F30(Func801C4F30Obj *o)
{
    func_801C4EB8(o);
    if (o->spin > 24)
        o->spin = 24;
    o->yaw += o->spin;
}
