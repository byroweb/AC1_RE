#include "ac1/ac1.h"

typedef struct {
    u8 r, g, b, cd;
} Rgba;

extern void func_800851A8(Projectile *);

/* kind-1 (straight, func_800851A8) projectile with explicit motion fields */
Projectile *func_800866F0(void *owner, AcPos *pos, SVECTOR *ang, s32 a, s32 b, Rgba *tint,
                          Rgba *dtint, SVECTOR *vel, SVECTOR *spin, s32 section)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, owner, pos, section);
    p->kind = 1;
    *(SVECTOR *)p->ang = *ang;
    *(s16 *)&p->unkc4[0] = a;
    *(s16 *)&p->unkc4[2] = b;
    *(Rgba *)p->tint = *tint;
    *(Rgba *)p->dtint = *dtint;
    *(SVECTOR *)p->vel = *vel;
    *(SVECTOR *)p->spin = *spin;
    p->think = func_800851A8;
    return p;
}
