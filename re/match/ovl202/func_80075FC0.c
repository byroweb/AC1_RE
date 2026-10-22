#include "ac1/ac1.h"

extern s32 func_80075EB4(AcEntity *e);
extern s16 func_800132CC(s32 cur, s32 want, s32 rate);

s32 func_80075FC0(AcEntity *e)
{
    s32 r;

    r = func_80075EB4(e);
    e->yaw = func_800132CC(e->yaw, (s16)r, ((s16 *)e->geom)[4]);
    return r;
}
