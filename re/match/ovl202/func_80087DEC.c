#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
} Vec3s;

typedef struct {
    s16 speed;                  /* +0x00 */
    s16 unk2;
    u16 life;                   /* +0x04 */
    u8 sfx;                     /* +0x06 */
    u8 unk7;
    s16 count;                  /* +0x08 */
    s16 jitter;                 /* +0x0A */
    u16 spread;                 /* +0x0C */
    u16 unke;
    s32 unk10;                  /* +0x10 */
    s32 unk14;                  /* +0x14 */
} MortarDef;

extern void func_80014774(SVECTOR *rot, SVECTOR *out);
extern s32 VectorNormalSS(SVECTOR *in, SVECTOR *out);
extern void func_8004CE44(s32 sfx, AcPos *pos, s32 vol, s32 arg3);
extern void func_80087BC8(Projectile *);
extern s32 rand(void);
extern MortarDef D_8008FC18[];

/* mortar / grenade volley: def->count shells thrown along the owner's facing,
 * inheriting a quarter of the owner's own motion */
Projectile *func_80087DEC(AcEntity *owner, AcPos *pos, s32 unused, Vec3s *a, s32 kind,
                          s32 unused2, SVECTOR *dir)
{
    AcEntity *e;
    MortarDef *def;
    Projectile *p;
    SVECTOR n;
    SVECTOR rot;
    SVECTOR out;
    SVECTOR v;
    s32 i;

    e = owner;
    rot = *(SVECTOR *)e->unk10;
    rot.vx = 256 - rot.vx;
    def = &D_8008FC18[kind];
    i = def->count;
    while (--i != -1) {
        p = proj_alloc(1);
        if (p == &g_proj_none) {
            return p;
        }
        proj_init(p, owner, pos, -1);
        v = *dir;
        v.vx = 0;
        p->kind = 2;
        p->ang[0] = 282;
        p->ang[1] = 56;
        p->ang[2] = 100;
        *(Vec3s *)p->dtint = *a;
        M2C_FIELD(p, s32 *, 0x30) = def->unk10;
        M2C_FIELD(p, s32 *, 0x34) = def->unk14;
        p->think = func_80087BC8;
        v = rot;
        v.vx += rand() & 0x1FF;
        v.vy = (v.vy + (def->spread + ((def->jitter >> 1) - (def->jitter & rand())))) & 0xFFF;
        func_80014774(&v, &out);
        VectorNormalSS(&out, &n);
        p->vel[1] = ((n.vx * def->speed) >> 12) + ((e->pos.x - e->prev_pos.x) >> 2);
        p->vel[2] = ((n.vy * def->speed) >> 12) + ((e->pos.y - e->prev_pos.y) >> 2);
        p->unk22 = ((n.vz * def->speed) >> 12) + ((e->pos.z - e->prev_pos.z) >> 2);
        M2C_FIELD(p, s16 *, 0x2C) = 0;
        p->unk12 = def->life;
    }
    func_8004CE44(def->sfx, pos, 90, 0);
    return p;
}
