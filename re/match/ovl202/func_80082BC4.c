#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x108];
    s16 h108;
    s16 h10a;
    s16 h10c;
    s16 h10e;
} AcTimers;

extern void func_800813D8(AcEntity *e, u8 state);
extern void func_8008238C(AcEntity *e, s32 a, s32 b);
extern void func_8007DB98(AcEntity *e);
extern void func_800826FC(AcEntity *e);

void func_80082BC4(AcEntity *e)
{
    e->fn_state = func_800813D8;
    e->fn_move = func_8008238C;
    e->fn_hit = func_8007DB98;
    e->fn_think = func_800826FC;
    ((AcTimers *)e)->h10c = 0;
    ((AcTimers *)e)->h108 = 0;
    ((AcTimers *)e)->h10a = 500;
    ((AcTimers *)e)->h10e = 0;
    if (e->fn_move != 0) {
        ((void (*)(AcEntity *, s32))e->fn_move)(e, 0);
    }
    if (e->fn_state != 0) {
        e->fn_state(e, e->state);
    }
}
