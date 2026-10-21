#include "ac1/ac1.h"

extern void func_800899E8(Projectile *);

Projectile *func_80089A80(AcPos *pos, SVECTOR *vel)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, 0, pos, -1);
    p->kind = 2;
    p->ang[0] = 0x11C;
    p->ang[1] = 0x100;
    p->ang[2] = 100;
    p->think = func_800899E8;
    *(SVECTOR *)p->spin = *vel;
    *(s16 *)&p->unk2a[0] = 4;
    return p;
}
