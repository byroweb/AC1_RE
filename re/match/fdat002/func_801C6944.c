#include "ac1/ac1.h"

typedef struct {
    void (*fn)();       /* +0x00 */
    s32 arg;            /* +0x04 */
} Func801C6944Hit;
typedef struct {
    u8 pad0[0x2C];
    Func801C6944Hit *on_hit; /* +0x2C */
} Func801C6944World;
extern Func801C6944Hit *func_80072120(s32 kind);
extern void func_801C68AC();

void func_801C6944(Func801C6944World *w)
{
    u8 unused[144];
    Func801C6944Hit *h;

    h = func_80072120(0);
    w->on_hit = h;
    h->fn = func_801C68AC;
    w->on_hit->arg = 0;
}
