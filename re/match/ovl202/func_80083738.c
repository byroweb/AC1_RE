#include "ac1/ac1.h"

typedef struct {
    u8 kind;                    /* +0 */
    u8 unk1;
    s16 damage;                 /* +2 */
    u8 impact;                  /* +4 */
} BlastDmg;

extern s32 SquareRoot0(s32 x);
extern AcEntity *func_8006D628(s32 *dist2);
extern AcEntity *func_8006D8F8(void *owner, Projectile *p, s32 a, s32 b, s32 *dist2);
extern void func_800742DC(AcEntity *e, SVECTOR *out);
extern void func_800835E8(void *owner, AcEntity *e, BlastDmg *d, SVECTOR *rel, SVECTOR *pos);
extern Projectile *func_80086854(void *owner, SVECTOR *pos, u32 *flags);
extern void func_800873F8(AcEntity *e, SVECTOR *pos, s32 a, s32 b, s32 c, s32 d);

/* splash damage: every object in range takes damage falling off linearly
 * with distance; each of the 16 AC slots is hit at most once (mask). */
s32 func_80083738(Projectile *p, s32 a, s32 b, s32 radius, BlastDmg *dmg, u32 *fx, u32 *bitsp,
                  s32 mask)
{
    SVECTOR pos;
    SVECTOR rel;
    BlastDmg d;
    s32 dist2;
    AcEntity *e;
    s32 bit;
    s32 k;
    u32 bits;

    e = func_8006D8F8(p->owner, p, a, b, &dist2);
    d.kind = dmg->kind;
    if (e != 0) {
        do {
            if (e >= g_ac_entities && e <= &g_ac_entities[15]) {
                bit = 1 << (e - g_ac_entities);
                if (mask & bit) {
                    continue;
                }
                mask |= bit;
            }
            func_800742DC(e, &pos);
            rel.vx = pos.vx - p->x;
            rel.vy = pos.vy - p->y;
            rel.vz = pos.vz - p->z;
            k = 0x1000 - (SquareRoot0(dist2) << 12) / radius;
            d.damage = (*(u16 *)&dmg->damage * k) >> 12;
            d.impact = (dmg->impact * k) >> 12;
            bits = *bitsp;
            func_800873F8(e, &pos, bits & 0x3FF, (bits >> 10) & 0xFF, (bits >> 18) & 0x1F, 1);
            func_800835E8(p->owner, e, &d, &rel, &pos);
            func_80086854(0, &pos, fx);
        } while ((e = func_8006D628(&dist2)) != 0);
    }
    return mask;
}
