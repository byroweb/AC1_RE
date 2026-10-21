#include "ac1/ac1.h"

Projectile *func_80085A14(void *owner, AcPos *pos, SVECTOR *a, s32 b, DVECTOR *c, s32 d)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, owner, pos, -1);
    p->kind = 1;
    *(SVECTOR *)p->ang = *a;
    *(DVECTOR *)p->tint = *c;
    *(s16 *)&p->unkc4[0] = b;
    *(s16 *)&p->unkc4[2] = d;
    p->think = 0;
    return p;
}
