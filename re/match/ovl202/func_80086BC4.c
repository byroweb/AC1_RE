#include "ac1/ac1.h"

extern void func_80085394(Projectile *);

Projectile *func_80086BC4(void *owner, AcPos *pos, s32 a, s32 b, s32 c, s32 d, s32 e)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p != &g_proj_none) {
        proj_init(p, owner, pos, -1);
        p->kind = 2;
        p->ang[0] = a;
        p->ang[1] = b;
        p->ang[2] = 0;
        p->vel[0] = e;
        *(s16 *)&p->dtint[0] = c;
        *(s16 *)&p->dtint[2] = d;
        p->think = func_80085394;
        func_80085394(p);
    }
    return p;
}
