/* fdat016 (16 twins): give a world object a timed trigger record (func_80072120(4)):
 * delay = stats +2 * 22, update func_801C7624, event hook func_801C76C0. */
#include "ac1/ac1.h"
typedef struct {
    void (*update)();   /* +0x00 */
    s32 (*event)();     /* +0x04 */
    u8 pad8[8];
    s32 delay;          /* +0x10 */
} Func801C76E4Hit;
typedef struct {
    u8 pad0[2];
    u16 secs;           /* +0x02 */
} Func801C76E4Stats;
typedef struct {
    u8 pad0[0x2C];
    Func801C76E4Hit *on_hit; /* +0x2C */
    u8 pad30[8];
    Func801C76E4Stats *stats; /* +0x38 */
} Func801C76E4World;
extern Func801C76E4Hit *func_80072120(s32 kind);
extern void func_801C7624();
extern s32 func_801C76C0();
void func_801C76E4(Func801C76E4World *w)
{
    u8 unused[144];
    Func801C76E4Hit *h;

    h = func_80072120(4);
    w->on_hit = h;
    h->delay = w->stats->secs * 22;
    w->on_hit->update = func_801C7624;
    w->on_hit->event = func_801C76C0;
}
