#include "ac1/ac1.h"

extern void func_8007EF94(AcEntity *e, u8 state);
extern void func_8008080C(AcEntity *e, s32 a, s32 b);
extern void func_8007DB98(AcEntity *e);
extern void func_80080D00(AcEntity *e);

void func_80080D20(AcEntity *e)
{
    e->fn_state = func_8007EF94;
    e->fn_move = func_8008080C;
    e->fn_hit = func_8007DB98;
    e->fn_think = func_80080D00;
    if (e->fn_move != 0) {
        ((void (*)(AcEntity *, s32))e->fn_move)(e, 0);
    }
    if (e->fn_state != 0) {
        e->fn_state(e, e->state);
    }
}
