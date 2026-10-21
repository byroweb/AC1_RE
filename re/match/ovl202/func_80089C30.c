#include "ac1/ac1.h"

typedef struct {
    u8 r, g, b, cd;
} Rgba;

extern void func_80089B44(Projectile *);
extern s32 rand(void);
extern Rgba D_8004C160;

Projectile *func_80089C30(AcPos *pos, SVECTOR *vel, s32 life)
{
    Projectile *p;
    Rgba col;

    p = proj_alloc(0);
    col = D_8004C160;
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, 0, pos, -1);
    p->kind = 6;
    p->ang[2] = (rand() & 1) + 0x118;
    p->unkbe = 0xB00;
    *(s16 *)p->tint = 100;
    *(Rgba *)p->ang = col;
    p->think = func_80089B44;
    *(SVECTOR *)p->spin = *vel;
    *(s16 *)&p->unk2a[0] = life;
    return p;
}
