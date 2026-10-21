#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Vec3s;

extern s32 VectorNormalSS(SVECTOR *in, SVECTOR *out);
extern void func_8004CE44(s32 sfx, AcPos *pos, s32 vol, s32 arg3);
extern void func_80083D38(Projectile *);
extern s32 func_80085AFC(Projectile *, void *, SVECTOR *, s32, s32, void *);
extern s32 rand(void);

Projectile *func_800860EC(void *owner, Vec3s *a, AcPos *pos, SVECTOR *dir, s32 speed, s32 t,
                          s32 b, s32 c, s32 e, s32 f, s32 *g, s32 *h, s32 jitter, s32 sfx)
{
    Projectile *p;
    SVECTOR n;
    s32 half;

    p = proj_alloc(1);
    if (p != &g_proj_none) {
        proj_init(p, owner, pos, -1);
        p->kind = 2;
        p->ang[0] = b;
        p->ang[1] = c;
        p->ang[2] = 100;
        VectorNormalSS(dir, &n);
        if (jitter != 0) {
            half = jitter >> 1;
            n.vx += (rand() & jitter) - half;
            n.vy += (rand() & jitter) - half;
            n.vz += (rand() & jitter) - half;
        }
        p->vel[2] = (n.vx * speed) >> 12;
        p->unk22 = (n.vy * speed) >> 12;
        p->spin[0] = (n.vz * speed) >> 12;
        *(Vec3s *)p->dtint = *a;
        M2C_FIELD(p, s32 *, 0x30) = *g;
        M2C_FIELD(p, s32 *, 0x34) = *h;
        p->vel[1] = e;
        M2C_FIELD(p, s16 *, 0x2E) = 0;
        p->think = func_80083D38;
        M2C_FIELD(p, s32 *, 0x38) = func_80085AFC(p, &p->vel[2], &n, f, t, &p->spin[2]);
        func_80083D38(p);
        func_8004CE44(sfx, pos, 90, 0);
    }
    return p;
}
