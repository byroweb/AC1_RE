#include "ac1/ac1.h"

void func_80085394(Projectile *p)
{
    p->ang[1] += *(s16 *)p->dtint;
    *(s16 *)p->dtint += *(s16 *)&p->dtint[2];
    if (p->ang[1] <= p->vel[0]) {
        p->unk12 = 0;
    }
}
