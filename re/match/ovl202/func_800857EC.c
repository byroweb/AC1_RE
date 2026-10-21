#include "ac1/ac1.h"

void func_800857EC(Projectile *p, s16 a, s16 b, SVECTOR *rot)
{
    p->kind = 0;
    p->unkdc = 0;
    *(SVECTOR *)p->ang = *rot;
    *(s16 *)&p->tint[0] = a;
    *(s16 *)&p->tint[2] = b;
    *(s16 *)&p->unkc4[4] = 0;
    *(s16 *)&p->unkc4[2] = 0;
    *(s16 *)&p->unkc4[0] = 0;
}
