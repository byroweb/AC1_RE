#include "ac1/ac1.h"
/* ovl203: fire up to n+1 shots of weapon e at the first locked target slot of the
 * side in D_80039D18; returns that slot (0 if none / no muzzle). */
typedef struct {
    u8 pad0[8];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x20 - 0x10];
    AcPos prev;         /* +0x20 */
} FObj3;
typedef struct {
    u8 pad0;
    u8 idx;             /* +0x01 */
    u8 pad2[0x10 - 2];
    u16 ammo;           /* +0x10 */
    u8 pad12[0x24 - 0x12];
    u8 shot[4];         /* +0x24 */
} FDef3;
typedef struct {
    u8 a, b;            /* +0x00 */
    u8 pad2[2];
    u16 spread_x;       /* +0x04 */
    u16 spread_y;       /* +0x06 */
} FEnt3;
typedef struct {        /* 0x30 */
    void *target;       /* +0x00 */
    u8 pad4[0x16 - 4];
    u16 flags;          /* +0x16 */
    s16 ax;             /* +0x18 */
    s16 ay;             /* +0x1A */
    u8 pad1c[0x26 - 0x1C];
    s16 n;              /* +0x26 */
    u8 pad28[8];
} TSlot3;
typedef struct {
    u8 pad0[0x24];
    s32 shot;           /* +0x24 */
    u8 pad28[0x80 - 0x28];
    TSlot3 *best;       /* +0x80 */
    TSlot3 slot[8];     /* +0x84 */
} Side3;
extern Side3 *D_80039D18;
extern s32 func_800732FC(FObj3 *o, SVECTOR *v, s32 a, s32 b, s32 c, s32 d, s32 e);
extern void func_80014AE4(s32 dx, s32 dy, s32 dz, SVECTOR *ang);
extern s32 func_80014D78(s32 a);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern void func_80014338(SVECTOR *v, s32 s, SVECTOR *out);
extern s32 func_80084F94(FObj3 *o, u8 *shot, SVECTOR *v, SVECTOR *ang, SVECTOR *vel, s32 a, s32 b, s32 c, void *t);

TSlot3 *func_8007B8CC(FObj3 *o, FDef3 *w, FEnt3 *e, s32 n, s32 p4, s32 p5)
{
    SVECTOR v[2];
    SVECTOR d;
    SVECTOR ang;
    SVECTOR a;
    SVECTOR dir;
    SVECTOR vel;
    TSlot3 *s;
    s32 i;
    s32 first;
    s32 r;

    first = 1;
    if (n == 0) {
        return 0;
    }
    s = D_80039D18->slot;
    i = 8;
    do {
        if (s->target != 0 && (s->flags & 8) && s->n != 0) {
            goto found;
        }
        s++;
        i--;
    } while (i != 0);
    return 0;
found:
    while (--n != -1) {
        if (w->ammo != 0) {
            w->ammo--;
            if (func_800732FC(o, v, p4, p5, 2, n * 2, n * 2 + 1) != 0) {
                func_80014AE4(v[1].vx - v[0].vx, v[1].vy - v[0].vy, v[1].vz - v[0].vz, &ang);
                d.vx = o->pos.x - o->prev.x;
                d.vy = (o->pos.y - o->prev.y) >> 2;
                d.vz = o->pos.z - o->prev.z;
                a.vx = (e->spread_x >> 1) - (ang.vx + (s->ax >> 2) + func_80014D78(e->spread_x));
                a.vy = ang.vy + (s->ay >> 2) + func_80014D78(e->spread_y) - (e->spread_y >> 1);
                a.vz = 0;
                if (w->idx == 6) {
                    a.vx += 192;
                }
                func_80014774(&a, &dir);
                func_80014338(&dir, 100, &vel);
                vel.vx += d.vx;
                vel.vy += d.vy;
                vel.vz += d.vz;
                a.pad = first;
                a.vx = -a.vx;
                r = func_80084F94(o, w->shot, v, &a, &vel, 48, e->a, e->b, s->target);
                if (D_80039D18->best == s) {
                    D_80039D18->shot = r;
                }
                first = 0;
            } else {
                return 0;
            }
        }
    }
    return s;
}
