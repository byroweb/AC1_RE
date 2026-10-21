#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Vec3s;

extern void func_80084EE0(Projectile *);
extern void func_800857EC(Projectile *p, s32 a, s32 b, SVECTOR *rot);
extern s32 func_80085AFC(Projectile *, void *, s32, s32, s32, void *);
extern s32 rand(void);

Projectile *func_80086598(void *owner, Vec3s *a, s32 *b, s32 *c, AcPos *pos, SVECTOR *rot,
                          SVECTOR *v, s32 kind, s32 arg8)
{
    Projectile *p;

    p = proj_alloc(1);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, owner, pos, -1);
    func_800857EC(p, kind, 0, rot);
    M2C_FIELD(p, s16 *, 0xC6) = rand() & 1;
    p->unkbe = kind != 0x37;
    *(Vec3s *)p->dtint = *a;
    M2C_FIELD(p, s32 *, 0x30) = *b;
    M2C_FIELD(p, s32 *, 0x34) = *c;
    p->spin[0] = 0;
    *(SVECTOR *)((u8 *)p + 0x1E) = *v;
    M2C_FIELD(p, s32 *, 0x38) = func_80085AFC(p, (u8 *)p + 0x1E, 0, 0, arg8, (u8 *)p + 0x26);
    p->think = func_80084EE0;
    return p;
}
