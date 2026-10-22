#include "ac1/ac1.h"

extern void func_801C61AC(AcEntity *, u8);
extern void func_801C6460(AcEntity *, s32, s32);
extern void func_801C6238(AcEntity *);

void func_801C6594(AcEntity *e)
{
    e->faction = 2;
    e->ignore_mask = 0x10;
    e->fn_state = func_801C61AC;
    e->fn_move = func_801C6460;
    e->fn_think = func_801C6238;
    if (e->fn_move != 0) {
        ((void (*)(AcEntity *, s32))e->fn_move)(e, 0);
    }
    if (e->fn_state != 0) {
        e->fn_state(e, e->state);
    }
}
