/* fdat000 (mission library, entry 0 @ 0x801C4B40; 33 twins): give a world object a
 * plain handler record (func_80072120(0)) updated by the "all destroyed" trigger
 * func_801C5428. */
#include "ac1/ac1.h"
typedef struct {
    void (*update)();   /* +0x00 */
} Func801C5550Hit;
typedef struct {
    u8 pad0[0x2C];
    Func801C5550Hit *on_hit; /* +0x2C */
} Func801C5550World;
extern Func801C5550Hit *func_80072120(s32 kind);
extern void func_801C5428();
void func_801C5550(Func801C5550World *w)
{
    Func801C5550Hit *h;

    h = func_80072120(0);
    w->on_hit = h;
    h->update = func_801C5428;
}
