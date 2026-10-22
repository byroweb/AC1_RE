#include "ac1/ac1.h"

typedef struct {
    void (*fn)();       /* +0x00 */
    void (*arg)();      /* +0x04 */
} Func801C730CHit;
typedef struct {
    u8 pad0[0x2C];
    Func801C730CHit *on_hit; /* +0x2C */
} Func801C730CWorld;
extern Func801C730CHit *func_80072120(s32 kind);
extern void func_801C72A8();

void func_801C730C(Func801C730CWorld *w)
{
    u8 unused[144];
    Func801C730CHit *h;

    h = func_80072120(1);
    w->on_hit = h;
    h->fn = 0;
    w->on_hit->arg = func_801C72A8;
}
