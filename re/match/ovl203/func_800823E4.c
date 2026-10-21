#include "ac1/ac1.h"

typedef struct {
    u8 type;                    /* +0x00 */
    u8 pad1;
    u16 power;                  /* +0x02 scaled by the falloff */
    u8 impact;                  /* +0x04 scaled by the falloff */
} BlastDef;

typedef struct {
    u8 type;                    /* +0x00 */
    u8 pad1;
    s16 power;                  /* +0x02 */
    u8 impact;                  /* +0x04 */
} BlastHit;

extern AcEntity D_8019B550[2];
extern long SquareRoot0(long a);
extern void *func_8006CD98(void *, Projectile *, s32, s32, s32 *);
extern void *func_8006CAC8(s32 *);
extern void func_80073008(void *, SVECTOR *);
extern void func_800860A4(void *, SVECTOR *, s32, s32, s32, s32);
extern void func_80082284(void *, void *, BlastHit *, SVECTOR *, SVECTOR *);
extern void func_80085500(s32, SVECTOR *, s32);

/* Splash damage: every object the area query returns gets a hit scaled by
 * 1 - dist / radius; each actor at most once per mask bit. */
s32 func_800823E4(Projectile *p, s32 a1, s32 a2, s32 radius, BlastDef *def, s32 arg5, u32 *info, s32 mask) {
    SVECTOR pos;
    SVECTOR d;
    BlastHit hit;
    s32 dist;
    void *o;
    s32 bit;
    s32 k;
    u32 v;

    o = func_8006CD98(p->owner, p, a1, a2, &dist);
    hit.type = def->type;
    while (o != 0) {
        if ((AcEntity *)o >= D_8019B550 && (AcEntity *)o <= &D_8019B550[1]) {
            bit = 1 << ((AcEntity *)o - D_8019B550);
            if (mask & bit) {
                goto next;
            }
            mask |= bit;
        }
        func_80073008(o, &pos);
        d.vx = pos.vx - p->x;
        d.vy = pos.vy - p->y;
        d.vz = pos.vz - p->z;
        k = 4096 - (SquareRoot0(dist) << 12) / radius;
        hit.power = (def->power * k) >> 12;
        hit.impact = (def->impact * k) >> 12;
        v = *info;
        func_800860A4(o, &pos, v & 0x3FF, (v >> 10) & 0xFF, (v >> 18) & 0x1F, 1);
        func_80082284(p->owner, o, &hit, &d, &pos);
        func_80085500(0, &pos, arg5);
    next:
        o = func_8006CAC8(&dist);
    }
    return mask;
}
