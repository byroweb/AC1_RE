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
    s16 jitter;                 /* +0x02 */
    u16 c6;                     /* +0x04 */
    u16 c4;                     /* +0x06 */
    s16 arc;                    /* +0x08 */
    u8 sfx;                     /* +0x0A */
    u8 unkb;
    Rgba col;                   /* +0x0C */
    s32 unk10;                  /* +0x10 */
    s32 unk14;                  /* +0x14 */
} ShellDef;

extern s32 VectorNormalSS(SVECTOR *in, SVECTOR *out);
extern void func_8004CE44(s32 sfx, AcPos *pos, s32 vol, s32 arg3);
extern s32 func_80085AFC(Projectile *, void *, SVECTOR *, s32, s32, void *);
extern void func_800880BC(Projectile *);
extern s32 rand(void);
extern ShellDef D_8008FB58[];

Projectile *func_8008840C(void *owner, AcPos *pos, SVECTOR *dir, Vec3s *a, s32 t, s32 kind)
{
    Projectile *p;
    ShellDef *def;
    SVECTOR n;

    p = proj_alloc(1);
    if (p != &g_proj_none) {
        proj_init(p, owner, pos, -1);
        *(Vec3s *)p->dtint = *a;
        def = &D_8008FB58[kind];
        M2C_FIELD(p, s32 *, 0x30) = def->unk10;
        M2C_FIELD(p, s32 *, 0x34) = def->unk14;
        p->think = func_800880BC;
        p->kind = 1;
        *(AcPos *)p->ang = *pos;
        *(Rgba *)p->tint = def->col;
        *(u16 *)&p->unkc4[0] = def->c4;
        *(u16 *)&p->unkc4[2] = def->c6;
        VectorNormalSS(dir, &n);
        if (def->jitter != 0) {
            n.vx += (def->jitter & rand()) - (def->jitter >> 1);
            n.vy += (def->jitter & rand()) - (def->jitter >> 1);
            n.vz += (def->jitter & rand()) - (def->jitter >> 1);
        }
        p->vel[1] = (n.vx * def->speed) >> 12;
        p->vel[2] = (n.vy * def->speed) >> 12;
        p->unk22 = (n.vz * def->speed) >> 12;
        M2C_FIELD(p, s16 *, 0x2C) = 0;
        M2C_FIELD(p, s32 *, 0x38) = func_80085AFC(p, &p->vel[1], &n, def->arc, t, &p->spin[1]);
        p->spin[0] = 2;
        func_8004CE44(def->sfx, pos, 127, 0);
    }
    return p;
}
