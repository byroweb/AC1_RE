#include "ac1/ac1.h"

#define ABS(x) ((x) >= 0 ? (x) : -(x))

extern long SquareRoot0(long a);
extern s32 func_800749A0(AcEntity *e, AcPos *pos, SVECTOR *tmp, SVECTOR *v);

s32 func_80075108(AcEntity *e, SVECTOR *v)
{
    SVECTOR save;
    SVECTOR tmp;
    s32 n;

    v->vy += SquareRoot0(v->vx * v->vx + v->vz * v->vz) >> 1;
    save = *v;
    n = func_800749A0(e, &e->pos, &tmp, v);
    if (v->vy <= 0) {
        if (ABS(v->vx) < ABS(save.vx) >> 1 || ABS(v->vz) < ABS(save.vz) >> 1) {
            *v = save;
            v->vy = 0;
            n = func_800749A0(e, &e->pos, &tmp, v);
            if (e->fn_move != 0) {
                e->fn_move(e, 5, n);
            }
        }
    }
}
