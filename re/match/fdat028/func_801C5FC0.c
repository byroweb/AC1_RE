#include "ac1/ac1.h"

typedef struct Func801C5FC0Obj {
    u8 pad0[0x2C];
    s16 x2C;            /* +0x2C */
    u8 pad2E[8];
    s16 mode;           /* +0x36 */
    u8 pad38[0x18];
    s32 x50;            /* +0x50 */
    void (*init)(struct Func801C5FC0Obj *, s32); /* +0x54 */
    void (*step)();     /* +0x58 */
    void (*hit)();      /* +0x5C */
    u8 pad60[4];
    u8 x64[0x18];       /* +0x64 */
    s16 x7C;            /* +0x7C */
} Func801C5FC0Obj;
extern void func_8004E98C(void *, s32, s32, s32);
extern void func_801C5F3C(Func801C5FC0Obj *, s32);
extern void func_801C5DCC();
extern void func_801C5F6C();

void func_801C5FC0(Func801C5FC0Obj *o)
{
    void (*f)(Func801C5FC0Obj *, s32);

    o->mode = 1;
    o->init = func_801C5F3C;
    f = *(void (*volatile *)(Func801C5FC0Obj *, s32))&o->init;
    o->step = func_801C5DCC;
    o->x50 = 0;
    o->hit = func_801C5F6C;
    o->x7C = 0;
    if (f != 0) {
        f(o, 0);
    }
    func_8004E98C(o->x64, 0, 0, 0);
}
