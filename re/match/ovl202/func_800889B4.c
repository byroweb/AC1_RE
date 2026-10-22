#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Vec3s;

typedef struct {
    s16 speed;                  /* +0x00 */
    s16 jitter;                 /* +0x02 */
    u16 ang1;                   /* +0x04 */
    u16 ang0;                   /* +0x06 */
    s16 arc;                    /* +0x08 */
    u8 sfx;                     /* +0x0A */
    u8 unkb;
    u32 unkc : 16;              /* +0x0C */
    u32 count : 8;              /* +0x0E shells per shot */
    u32 fx : 2;
    u32 fxidx : 6;
    u8 unk10[0x0C];
    DVECTOR fxcol;              /* +0x1C */
    s32 unk20;                  /* +0x20 */
    s32 unk24;                  /* +0x24 */
} SpreadDef;

typedef struct {
    s16 a;
    s16 b;
} FxPair;

extern s32 VectorNormalSS(SVECTOR *in, SVECTOR *out);
extern void func_8004CE44(s32 sfx, AcPos *pos, s32 vol, s32 arg3);
extern Projectile *func_80085948(void *owner, void *pos, s32 a, s32 b, s32 c, s32 section, DVECTOR *d);
extern s32 func_80085AFC(Projectile *, void *, SVECTOR *, s32, s32, void *);
extern void func_80088664(Projectile *);
extern s32 rand(void);
extern SpreadDef D_8008FC48[];
extern FxPair D_8008FEC8[];

/* weapon ids 20..39 (0x28 table): def->count shells per shot; returns the
 * none slot when the pool runs out, the success path falls off the end */
Projectile *func_800889B4(void *owner, AcPos *pos, SVECTOR *dir, Vec3s *a, s32 t, s32 kind)
{
    SpreadDef *def;
    Projectile *p;
    SVECTOR n;
    s32 i;

    def = &D_8008FC48[kind];
    for (i = 0; i < (s32)def->count; i++) {
        p = proj_alloc(1);
        if (p == &g_proj_none) {
            return p;
        }
        proj_init(p, owner, pos, -1);
        p->vel[1] = kind;
        *(Vec3s *)p->dtint = *a;
        M2C_FIELD(p, s32 *, 0x30) = def->unk20;
        M2C_FIELD(p, s32 *, 0x34) = def->unk24;
        p->spin[1] = t / def->speed;
        p->think = func_80088664;
        p->kind = 2;
        p->ang[0] = def->ang0;
        p->ang[1] = def->ang1;
        p->ang[2] = 100;
        VectorNormalSS(dir, &n);
        if (def->jitter > 0) {
            n.vx += (def->jitter & rand()) - (def->jitter >> 1);
            n.vy += (def->jitter & rand()) - (def->jitter >> 1);
            n.vz += (def->jitter & rand()) - (def->jitter >> 1);
        }
        p->vel[2] = (n.vx * def->speed) >> 12;
        p->unk22 = (n.vy * def->speed) >> 12;
        p->spin[0] = (n.vz * def->speed) >> 12;
        M2C_FIELD(p, s16 *, 0x2E) = 0;
        M2C_FIELD(p, s32 *, 0x38) = func_80085AFC(p, &p->vel[2], &n, def->arc, t, &p->spin[2]);
        if (def->fx) {
            func_80085948(0, p, D_8008FEC8[def->fxidx].a, D_8008FEC8[def->fxidx].b, 100, p->section,
                          &def->fxcol)->unk12 = 0;
        }
    }
    func_8004CE44(def->sfx, pos, 100, 0);
}
