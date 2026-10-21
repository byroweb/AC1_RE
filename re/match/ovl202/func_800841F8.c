#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    u8 unk0[4];
    u16 a;                      /* +0x04 */
    u16 b;                      /* +0x06 */
    u8 unk8[0x10];
    u8 c[3];                    /* +0x18 */
    u8 life;                    /* +0x1B */
    u8 d[4];                    /* +0x1C */
} BeamDef;

extern Projectile *func_800866F0(s32, Projectile *, SVECTOR *, s32, s32, u8 *, u8 *, SVECTOR *, SVECTOR *, s32);

/* chain the next beam segment of p; dir is reversed in place */
void func_800841F8(Projectile *p, BeamDef *def, SVECTOR *pos, s32 unused, SVECTOR *dir)
{
    Projectile *prev;
    Projectile *seg;
    SVECTOR *v;

    dir->vx = -dir->vx;
    dir->vy = -dir->vy;
    dir->vz = -dir->vz;
    prev = M2C_FIELD(p, Projectile **, 0x30);
    if (prev == 0) {
        v = dir;
    } else {
        v = (SVECTOR *)prev->vel;
        *pos = *(SVECTOR *)prev;
        if (p < prev) {
            pos->vx += prev->vel[0];
            pos->vy += prev->vel[1];
            pos->vz += prev->vel[2];
        }
    }
    seg = func_800866F0(0, p, pos, def->a, def->b, def->c, def->d, dir, v, p->section);
    M2C_FIELD(p, Projectile **, 0x30) = seg;
    seg->unk12 = def->life;
}
