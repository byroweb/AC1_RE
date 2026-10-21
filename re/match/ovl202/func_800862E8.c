#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Vec3s;

typedef struct {
    u8 unk0[6];
    u16 life;                   /* +0x06 */
    u8 unk8[0x20];
} Def28;

extern void func_800742DC(s32 target, void *out);
extern void func_80084314(Projectile *);
extern void func_800857EC(Projectile *p, s32 a, s32 b, SVECTOR *rot);
extern s32 rand(void);
extern Def28 D_80090198[];

Projectile *func_800862E8(void *owner, Vec3s *a, AcPos *pos, SVECTOR *rot, SVECTOR *v, s32 b,
                          s32 kind, s32 c, s32 target)
{
    Projectile *p;

    p = proj_alloc(1);
    if (p == &g_proj_none) {
        return &g_proj_none;
    }
    proj_init(p, owner, pos, -1);
    func_800857EC(p, b, 0, rot);
    *(SVECTOR *)((u8 *)p + 0x1E) = *v;
    *(Vec3s *)p->dtint = *a;
    M2C_FIELD(p, s16 *, 0x3C) = kind;
    M2C_FIELD(p, s32 *, 0x34) = target;
    M2C_FIELD(p, s32 *, 0x30) = 0;
    M2C_FIELD(p, s16 *, 0x38) = 0;
    M2C_FIELD(p, s16 *, 0x3A) = c;
    M2C_FIELD(p, s16 *, 0x3E) = 0;
    func_800742DC(target, (u8 *)p + 0x26);
    M2C_FIELD(p, s16 *, 0xC6) = rand() & 1;
    p->unk12 = D_80090198[kind].life;
    p->think = func_80084314;
    return p;
}
