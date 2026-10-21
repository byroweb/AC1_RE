#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Vec3s;

typedef struct {
    u8 r, g, b, cd;
} Rgba;

typedef struct {
    s16 speed;                  /* +0x00 */
    u16 size;                   /* +0x02 */
    u8 unk4[2];
    s16 model;                  /* +0x06 */
    u8 sfx;                     /* +0x08 */
    u8 unk9[0x13];
    s32 unk1c;                  /* +0x1C */
    s32 unk20;                  /* +0x20 */
} MissileDef24;

extern s32 VectorNormalSS(SVECTOR *in, SVECTOR *out);
extern void func_8004CE44(s32 sfx, AcPos *pos, s32 vol, s32 arg3);
extern void func_800857EC(Projectile *p, s32 a, s32 b, SVECTOR *rot);
extern void func_8008582C(Projectile *p, s32 a, SVECTOR *u, SVECTOR *v, Rgba *w);
extern s32 func_80085AFC(Projectile *, void *, SVECTOR *, s32, s32, void *);
extern void func_80088C80(Projectile *);
extern s32 rand(void);
extern MissileDef24 D_8008FEE8[];
extern Rgba D_8004C158;

/* weapon ids 40.. (0x24 table): guided missile */
Projectile *func_8008915C(void *owner, AcPos *pos, SVECTOR *dir, Vec3s *a, s32 speed, s32 kind,
                          s32 unused, SVECTOR *rot)
{
    MissileDef24 *def;
    Projectile *p;
    SVECTOR n;
    SVECTOR sc;
    Rgba col;

    def = &D_8008FEE8[kind];
    p = proj_alloc(1);
    if (p == &g_proj_none) {
        return p;
    }
    proj_init(p, owner, pos, -1);
    if (kind < 6) {
        func_800857EC(p, def->model, 0, rot);
        M2C_FIELD(p, s16 *, 0xC6) = rand() & 1;
    } else {
        col = D_8004C158;
        sc.vx = sc.vy = sc.vz = def->size;
        func_8008582C(p, def->model, rot, &sc, &col);
        p->unk11 = 1;
    }
    p->vel[1] = kind;
    *(Vec3s *)p->dtint = *a;
    p->spin[1] = speed / def->speed;
    p->think = func_80088C80;
    M2C_FIELD(p, s32 *, 0x30) = def->unk1c;
    M2C_FIELD(p, s32 *, 0x34) = def->unk20;
    VectorNormalSS(dir, &n);
    p->vel[2] = (n.vx * def->speed) >> 12;
    p->unk22 = (n.vy * def->speed) >> 12;
    p->spin[0] = (n.vz * def->speed) >> 12;
    M2C_FIELD(p, s16 *, 0x2E) = 0;
    M2C_FIELD(p, s32 *, 0x38) = func_80085AFC(p, &p->vel[2], &n, 0, speed, &p->spin[2]);
    func_8004CE44(def->sfx, pos, 100, 0);
}
