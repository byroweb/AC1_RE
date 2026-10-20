/* fdat002 (16 twins): spin step.  Advance the mover at +0x64 (func_8004ECA8); when the
 * +0x66 countdown reaches 0 call the +0x50 hook; then add the spin acceleration (+0xCE)
 * to the spin (+0xCC). */
#include "ac1/ac1.h"
typedef struct Func801C4EB8Obj Func801C4EB8Obj;
struct Func801C4EB8Obj {
    u8 pad0[0x12];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x41 - 0x14];
    u8 b41;             /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn)(Func801C4EB8Obj *, s32); /* +0x50 */
    u8 pad54[0x64 - 0x54];
    u8 mover[2];        /* +0x64 */
    u8 count;           /* +0x66 */
    u8 pad67[0xCC - 0x67];
    s16 spin;           /* +0xCC */
    s16 spin_acc;       /* +0xCE */
};
extern void func_8004ECA8(u8 *mover);
void func_801C4EB8(Func801C4EB8Obj *o)
{
    func_8004ECA8(o->mover);
    if (--o->count == 0) {
        if (o->fn != 0)
            o->fn(o, o->b41);
    }
    o->spin += o->spin_acc;
}
