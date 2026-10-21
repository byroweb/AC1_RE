#include "ac1/ac1.h"

typedef struct {
    u16 life;
    u16 phase;
    u16 amp;
    u16 step;
} WaveDef;

extern void func_80085328(Projectile *);
extern WaveDef D_800906C4[];

Projectile *func_80086AD8(void *owner, AcPos *pos, s32 a, s32 kind, s32 b)
{
    Projectile *p;
    WaveDef *w;

    p = proj_alloc(0);
    if (p != &g_proj_none) {
        proj_init(p, owner, pos, -1);
        p->kind = 2;
        w = &D_800906C4[kind];
        p->ang[0] = a;
        p->ang[1] = 0;
        p->ang[2] = 0;
        p->dtint[0] = 0;
        p->vel[1] = b;
        p->dtint[1] = kind;
        *(u16 *)&p->dtint[2] = w->phase;
        p->vel[0] = w->amp;
        p->unk12 = w->life;
        p->think = func_80085328;
        func_80085328(p);
    }
    return p;
}
