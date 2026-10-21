#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Vec3s;

typedef struct {
    s16 a;
    s16 b;
    s16 glow;
    u8 c;
    u8 d;
    u8 e;
    u8 col[5];
} MissileDef;

extern void func_8004CE44(s32 sfx, AcPos *pos, s32 vol, s32 arg3);
extern void func_80084A98(SVECTOR *v, s32 a, s32 b, void *out);
extern void func_80084DAC(Projectile *);
extern void func_800857EC(Projectile *p, s32 a, s32 b, SVECTOR *rot);
extern s32 rand(void);
extern MissileDef D_80090354[];

Projectile *func_80086448(void *owner, Vec3s *a, AcPos *pos, SVECTOR *rot, SVECTOR *v, s32 b,
                          s32 kind, s32 life)
{
    Projectile *p;
    MissileDef *def;

    p = proj_alloc(1);
    if (p != &g_proj_none) {
        proj_init(p, owner, pos, -1);
        func_800857EC(p, b, 0, rot);
        *(SVECTOR *)((u8 *)p + 0x1E) = *v;
        *(Vec3s *)p->dtint = *a;
        M2C_FIELD(p, s16 *, 0x2E) = kind;
        p->unk12 = life;
        M2C_FIELD(p, s16 *, 0xC6) = rand() & 1;
        def = &D_80090354[kind];
        func_80084A98(v, def->a, def->b, (u8 *)p + 0x26);
        p->think = func_80084DAC;
        func_8004CE44(0, pos, 120, 0);
    }
    return p;
}
