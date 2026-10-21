#include "ac1/ac1.h"

void func_8008582C(Projectile *p, s16 a, SVECTOR *u, SVECTOR *v, DVECTOR *w)
{
    p->kind = 4;
    p->ang[0] = a;
    p->ang[1] = 0;
    *(SVECTOR *)&p->ang[2] = *u;
    *(SVECTOR *)&p->unkc4[0] = *v;
    *(DVECTOR *)&p->unkc4[8] = *w;
}
