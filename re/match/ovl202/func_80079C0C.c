#include "ac1/ac1.h"

extern u16 D_800411B4;
extern u16 D_800411B6;
extern void func_80079798(AcEntity *e, s32 a, s32 b);
extern s32 func_8004F080(AcMover *m, AcPos *pos);

void func_80079C0C(AcEntity *e)
{
    func_80079798(e, D_800411B4, D_800411B6);
    if (func_8004F080(&e->mover, &e->pos) == 0 && e->fn_state != 0) {
        e->fn_state(e, e->state);
    }
}
