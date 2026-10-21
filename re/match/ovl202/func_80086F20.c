#include "ac1/ac1.h"

typedef struct {
    u8 r, g, b, cd;
} Rgba;

extern void func_80085454(Projectile *);

Projectile *func_80086F20(void *owner, AcPos *pos, s32 a, s32 b, s32 e, SVECTOR *u, SVECTOR *v,
                          Rgba *col, DVECTOR *w)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, owner, pos, -1);
    p->kind = 6;
    p->ang[2] = a;
    p->unkbe = b;
    *(s16 *)p->tint = 100;
    *(Rgba *)p->ang = *col;
    *(SVECTOR *)p->dtint = *u;
    p->vel[1] = e;
    *(SVECTOR *)&p->vel[2] = *v;
    *(DVECTOR *)&p->spin[2] = *w;
    p->think = func_80085454;
    return p;
}
