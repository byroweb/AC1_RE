#include "ac1/ac1.h"

typedef struct {
    u8 r, g, b, cd;
} Rgba;

extern void func_8008938C(Projectile *);
extern Rgba D_8004C15C;

/* returns the pool's none slot when full; the success path falls off the end */
Projectile *func_80089484(AcPos *pos, SVECTOR *vel, s32 a, s32 b, s32 c, s32 d)
{
    Projectile *p;
    Rgba col;

    col = D_8004C15C;
    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return p;
    }
    proj_init(p, 0, pos, -1);
    p->kind = 6;
    p->unkbe = b * 3;
    p->ang[2] = c;
    *(s16 *)p->tint = 100;
    *(Rgba *)p->ang = col;
    *(SVECTOR *)p->spin = *vel;
    *(s16 *)&p->unk2a[0] = 12;
    *(s16 *)&p->dtint[2] = a;
    p->vel[0] = b;
    p->vel[1] = d;
    p->think = func_8008938C;
}
