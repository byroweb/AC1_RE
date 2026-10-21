#include "ac1/ac1.h"

extern void func_8004CC30(s32, s32, AcPos *, s32, s32, s32, s32);
extern void func_800853D4(Projectile *);

Projectile *func_80086C9C(void *owner, AcPos *pos, s32 a, s32 b, s32 c, s32 d, s32 e,
                          s32 vx, s32 vy, s32 vz, s32 cue, s32 cue2)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, owner, pos, -1);
    if (cue != -1) {
        func_8004CC30(cue, cue2, pos, (e >> 3) + 50, 5333, 6666, 16 - (e >> 3));
    }
    p->kind = 5;
    p->ang[0] = a;
    p->ang[1] = b;
    p->ang[2] = c;
    p->unkbe = 100;
    *(s16 *)&p->dtint[0] = d;
    *(s16 *)&p->dtint[2] = e;
    p->vel[0] = vx;
    p->vel[1] = vy;
    p->vel[2] = vz;
    p->think = func_800853D4;
    return p;
}
