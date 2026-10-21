#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Vec3s;

extern void func_8004CE44(s32 sfx, AcPos *pos, s32 vol, s32 arg3);
extern void func_800857EC(Projectile *p, s16 a, s16 b, SVECTOR *rot);
extern void func_80089758(Projectile *);
extern s32 rand(void);

Projectile *func_800898A4(Vec3s *a, s32 *b, s32 *c, AcPos *pos, SVECTOR *v, SVECTOR *rot, s32 life)
{
    Projectile *p;
    s32 big;

    p = proj_alloc(1);
    if (p == &g_proj_none) {
        return p;
    }
    proj_init(p, 0, pos, -1);
    big = (rand() >> 10) >= 30;
    func_800857EC(p, big + 57, 0, rot);
    if (big) {
        func_8004CE44(51, pos, 90, 300);
    }
    *(Vec3s *)p->dtint = *a;
    M2C_FIELD(p, s32 *, 0x30) = *b;
    M2C_FIELD(p, s32 *, 0x34) = *c;
    *(SVECTOR *)((u8 *)p + 0x1E) = *v;
    p->unk12 = life;
    p->think = func_80089758;
    return p;
}
