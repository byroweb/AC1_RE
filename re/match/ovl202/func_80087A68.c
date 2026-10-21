#include "ac1/ac1.h"

typedef struct {
    u8 r, g, b, cd;
} Rgba;

extern void func_80087A24(Projectile *);
extern s32 rand(void);

Projectile *func_80087A68(AcPos *pos, s32 a, s32 b, s32 c, s32 life, Rgba *col, DVECTOR *w,
                          s32 jitter)
{
    Projectile *p;
    s32 half;

    p = proj_alloc(0);
    if (p == &g_proj_none) {
        return p;
    }
    proj_init(p, 0, pos, -1);
    p->kind = 6;
    p->ang[2] = a;
    p->unkbe = b;
    *(s16 *)p->tint = 100;
    *(Rgba *)p->ang = *col;
    if (jitter != 0) {
        half = jitter >> 1;
        p->x += (rand() & jitter) - half;
        p->y += (rand() & jitter) - half;
        p->z += (rand() & jitter) - half;
    }
    *(DVECTOR *)&p->spin[2] = *w;
    p->vel[1] = c;
    p->unk12 = life;
    p->think = func_80087A24;
    return p;
}
