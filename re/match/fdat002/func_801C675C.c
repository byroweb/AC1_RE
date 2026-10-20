/* fdat002 (39 twins): give a world object a plain handler record (func_80072120(0))
 * with the event hook func_801C6504. */
#include "ac1/ac1.h"
typedef struct {
    void (*update)();   /* +0x00 */
    s32 (*event)();     /* +0x04 */
} Func801C675CHit;
typedef struct {
    u8 pad0[0x2C];
    Func801C675CHit *on_hit; /* +0x2C */
} Func801C675CWorld;
extern Func801C675CHit *func_80072120(s32 kind);
extern s32 func_801C6504();
void func_801C675C(Func801C675CWorld *w)
{
    Func801C675CHit *h;

    h = func_80072120(0);
    w->on_hit = h;
    h->event = func_801C6504;
}
