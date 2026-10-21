#include "ac1/ac1.h"

Projectile *func_80085948(void *owner, AcPos *pos, s32 a, s32 b, s32 c, s32 section, DVECTOR *d)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, owner, pos, section);
    p->kind = 6;
    p->ang[2] = a;
    p->unkbe = b;
    *(s16 *)p->tint = c;
    *(DVECTOR *)p->ang = *d;
    p->think = 0;
    return p;
}
