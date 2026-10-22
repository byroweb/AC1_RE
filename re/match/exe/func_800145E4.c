#include "ac1/ac1.h"
#include "ac1/gte.h"

void func_800145E4(SVECTOR *v, s32 t)
{
    gte_lddp(t);
    gte_ldsv(v);
    gte_gpf12();
    gte_stsv(v);
}
