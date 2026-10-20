/* fdat000 (mission library, entry 0 @ 0x801C4B40; 21 twins): give a world object a
 * hit-handler record: allocate one (func_80072120(2)), store its world index at +0x10
 * and the handler func_801C5314 at +0. */
#include "ac1/ac1.h"
typedef struct {
    void (*fn)();       /* +0x00 */
    u8 pad4[0x0C];
    s16 world;          /* +0x10 */
} Func801C53D4Hit;
typedef struct {
    u8 pad0[0x2C];
    Func801C53D4Hit *on_hit; /* +0x2C */
    u8 pad30[0x10];
} Func801C53D4World;
extern Func801C53D4World D_801D0B68[];
extern Func801C53D4Hit *func_80072120(s32 kind);
extern void func_801C5314();
void func_801C53D4(Func801C53D4World *w)
{
    u8 unused[144];
    Func801C53D4Hit *h;

    h = func_80072120(2);
    w->on_hit = h;
    h->world = w - D_801D0B68;
    w->on_hit->fn = func_801C5314;
}
