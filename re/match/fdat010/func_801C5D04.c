#include "ac1/ac1.h"

typedef struct {
    void (*fn)();       /* +0x00 */
    void (*arg)();      /* +0x04 */
} Func801C5D04Hit;
typedef struct {
    u8 pad0[0x2C];
    Func801C5D04Hit *on_hit; /* +0x2C */
} Func801C5D04World;
extern Func801C5D04Hit *func_80072120(s32 kind);
extern void func_801C5C6C();

void func_801C5D04(Func801C5D04World *w)
{
    u8 unused[144];
    Func801C5D04Hit *h;

    h = func_80072120(0);
    w->on_hit = h;
    h->arg = func_801C5C6C;
}
