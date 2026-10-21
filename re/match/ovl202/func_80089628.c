#include "ac1/ac1.h"

extern void func_8004CE44(s32 sfx, AcPos *pos, s32 vol, s32 arg3);
extern void func_800895B0(Projectile *);

Projectile *func_80089628(AcPos *pos, SVECTOR *vel, s32 life, s32 kind)
{
    Projectile *p;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, 0, pos, -1);
    p->free = 1;
    *(SVECTOR *)p->spin = *vel;
    p->think = func_800895B0;
    *(s16 *)&p->unk2a[0] = life;
    switch (kind) {
    case 1:
        p->vel[0] = 20;
        *(s16 *)&p->dtint[2] = 1;
        p->vel[1] = 8;
        p->vel[2] = 0;
        func_8004CE44(52, pos, 120, 0);
        break;
    case 3:
        p->vel[0] = 14;
        p->vel[1] = 294;
        *(s16 *)&p->dtint[2] = 0;
        p->vel[2] = 10;
        func_8004CE44(52, pos, 80, 0);
        break;
    }
    return p;
}
