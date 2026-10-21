#include "ac1/ac1.h"

extern void func_80089D34(Projectile *);
typedef struct { u8 r, g, b, cd; } Rgba;
extern Rgba D_80090798;

Projectile *func_80089DE4(AcPos *pos, SVECTOR *vel)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, 0, pos, -1);
    p->kind = 6;
    p->ang[2] = 0x11B;
    p->unkbe = 0x50;
    *(s16 *)p->tint = 100;
    *(Rgba *)p->ang = D_80090798;
    p->unk11 = 1;
    p->think = func_80089D34;
    *(SVECTOR *)p->spin = *vel;
    *(s16 *)&p->unk2a[0] = 20;
    return p;
}
