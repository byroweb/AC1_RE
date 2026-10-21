#include "ac1/ac1.h"

Projectile *func_8008589C(void *owner, AcPos *pos, s32 a, s32 b, s32 c, s32 section)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, owner, pos, section);
    p->kind = 2;
    p->ang[0] = a;
    p->ang[1] = b;
    p->ang[2] = c;
    p->think = 0;
    return p;
}
