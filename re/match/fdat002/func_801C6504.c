/* fdat002 (36 twins): world-object event hook (installed by func_801C675C),
 * func(w, event, a, thing).  Event 0x8000 is a hit by `thing`: its damage (+2) is
 * taken off the stats' hit points (+0, 0xFFFF = already destroyed).  When they run
 * out, the object is destroyed at its box centre: sound +4 (func_8004CE44), an
 * explosion of size +3 (func_800869F0, with the shared angle bitfield at 0x801C6DDC /
 * 0x801C6DE0), debris +2 (func_800873F8) and effect +0x0C (func_80087798, 99 = none
 * and keep no handler), then func_8006D45C decides whether the mission event
 * func_8008A0B0 fires.  Always returns 0.  The hit points are loaded into hp and
 * decremented in place (one pseudo, as retail's register reuse needs). */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[2];
    u16 damage;         /* +0x02 */
} Func801C6504Thing;
typedef struct {
    u16 hp;             /* +0x00 0xFFFF = destroyed */
    u8 debris;          /* +0x02 */
    u8 blast;           /* +0x03 */
    u8 sound;           /* +0x04 */
    u8 volume;          /* +0x05 */
    u8 a;               /* +0x06 */
    u8 b;               /* +0x07 */
    u16 c;              /* +0x08 */
    u8 d;               /* +0x0A */
    u8 e;               /* +0x0B */
    u8 effect;          /* +0x0C 99 = none, drop the handler */
    u8 effect_b;        /* +0x0D */
    u16 effect_c;       /* +0x0E */
} Func801C6504Stats;
typedef struct {
    u8 pad0[0x10];
    SVECTOR pos;        /* +0x10 */
    s16 min[4];         /* +0x18 */
    s16 max[4];         /* +0x20 */
    u8 pad28[4];
    void *on_hit;       /* +0x2C */
    u8 pad30[8];
    Func801C6504Stats *stats; /* +0x38 */
} Func801C6504World;
typedef struct {
    u8 kind;            /* +0x00 */
    u8 pad1;
    u16 c;              /* +0x02 */
    u8 b;               /* +0x04 */
} Func801C6504Fx;
typedef struct {
    u32 lo : 17;
    u32 angle : 10;
    u32 hi : 5;
} Func801C6504Bits;
extern Func801C6504Bits D_801C6DDC;
extern Func801C6504Bits D_801C6DE0;
extern void func_8004CE44(s32 sound, SVECTOR *pos, s32 vol, s32 d);
extern void func_800869F0(s32 a, SVECTOR *at, s32 angle, s32 n, Func801C6504Bits *bits);
extern void func_800873F8(Func801C6504World *w, SVECTOR *pos, s32 a, s32 b, s32 n, s32 c);
extern void func_80087798(s32 a, SVECTOR *at, Func801C6504Fx *fx, s32 effect, s32 b);
extern s32 func_8006D45C(Func801C6504World *w);
extern void func_8006D39C(Func801C6504World *w);
extern s32 func_8008A0B0(s32 a, s32 b, s32 c, s32 d, s32 e);
s32 func_801C6504(Func801C6504World *w, s32 event, s32 a, ...)
{
    s32 v;
    Func801C6504Stats *s;
    Func801C6504Thing *t;
    SVECTOR mid;
    Func801C6504Fx fx;
    s32 hp;

    if (event == 0x8000) {
        s = w->stats;
        hp = s->hp;
        if (hp != 0xFFFF) {
            t = *(Func801C6504Thing **)((char *)&a + 4);
            hp -= t->damage;
            if (hp > 0)
                goto alive;
            mid.vx = (w->min[0] + w->max[0]) >> 1;
            mid.vy = (w->min[1] + w->max[1]) >> 1;
            mid.vz = (w->min[2] + w->max[2]) >> 1;
            s->hp = 0xFFFF;
            if (s->sound != 0)
                func_8004CE44(s->sound, &w->pos, s->volume, 0);
            if (s->blast != 0) {
                v = s->blast * 100;
                D_801C6DE0.angle = v;
                D_801C6DDC.angle = v;
                func_800869F0(0, &mid, D_801C6DDC.angle & ~3, s->blast * 2 - 1, &D_801C6DDC);
            }
            if (s->debris != 0)
                func_800873F8(w, &w->pos, 30, 20, s->debris, 0);
            if (s->effect != 0 && s->effect != 99) {
                fx.kind = 23;
                fx.c = s->effect_c;
                fx.b = s->effect_b;
                func_80087798(0, &mid, &fx, s->effect, 1);
            }
            if (func_8006D45C(w)) {
                if (s->effect != 99)
                    func_8006D39C(w);
                else
                    w->on_hit = 0;
                func_8008A0B0(s->a, s->b, s->c, s->d, s->e);
            } else {
                if (s->effect != 99)
                    func_8006D39C(w);
                else
                    w->on_hit = 0;
            }
            return 0;
        alive:
            s->hp = hp;
        }
    }
    return 0;
}
