/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0015): fire
 * control.  Count the cool-down at +0x3F; when idle, aim (801C7B1C) and, once a
 * round is ready and the weapon's chance roll passes, queue the rounds at +0x3E.  Then
 * fire one shot (or the whole queue with flag 0x10) through func_80077B78, picking the
 * projectile from the weapon record and, for types below 10, a random spread from the
 * table at 0x8008E610.  A target with flag 8 may dodge (50%) and end the volley. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[2];
    u16 type;           /* +0x02 low nibble = spread type; 0x10 volley, 0x20 full clip,
                         *       0x40 alternate barrels, 0x80 barrel index */
    u8 pad4;
    u8 proj_a;          /* +0x05 */
    u8 sub_a;           /* +0x06 */
    u8 proj_b;          /* +0x07 */
    u8 sub_b;           /* +0x08 */
    u8 pad9[2];
    s8 recoil;          /* +0x0B */
    u8 padc[3];
    u8 chance;          /* +0x0F */
    u8 pad10[0x1C - 0x10];
    s16 rounds;         /* +0x1C */
    u8 pad1e[2];
    u8 cool;            /* +0x20 */
    u8 pad21;
    u8 b22;             /* +0x22 */
    u8 pad23;
    u16 h24;            /* +0x24 */
    u8 b26;             /* +0x26 */
} Func801C88ECWeapon;
typedef struct {
    u8 pad0[0x38];
    u16 flags;          /* +0x38 bit 3: may dodge */
} Func801C88ECTarget;
typedef struct {
    u8 pad0[0x3E];
    u8 queued;          /* +0x3E */
    u8 cool;            /* +0x3F */
    u8 pad40[0x48 - 0x40];
    Func801C88ECTarget *target; /* +0x48 */
    u8 pad4c[0x76 - 0x4C];
    s16 aim_x;          /* +0x76 */
    s16 aim_y;          /* +0x78 */
    s16 aim_z;          /* +0x7A */
    s16 recoil;         /* +0x7C */
    u8 pad7e[0x133 - 0x7E];
    u8 shots;           /* +0x133 */
    u8 rounds;          /* +0x134 */
} Func801C88ECObj;
typedef struct {
    u16 proj;           /* +0x00 */
    u16 sub;            /* +0x02 */
    u8 pad4[0x0A - 0x04];
    u16 a;              /* +0x0A */
    u16 b;              /* +0x0C */
    u16 c;              /* +0x0E */
    u8 pad10[0x20 - 0x10];
} Func801C88ECShot;
typedef struct {
    u8 a;
    u8 c;
    u8 pad2[2];
    u16 spread_x;
    u16 spread_y;
} Func801C88ECSpread;
extern Func801C88ECSpread D_8008E610[];
extern s32 func_801C7B1C(Func801C88ECObj *o, Func801C88ECWeapon *w, s32 flags, void *arg);
extern s32 func_8002A4A0(void);
extern s32 func_80014D78(s32 n);
extern void func_80077B78();
void func_801C88EC(Func801C88ECObj *o, Func801C88ECWeapon *w)
{
    Func801C88ECShot shot;
    Func801C88ECSpread *sp;
    s32 n, side, dx, kind;

    o->aim_z = 0;
    o->aim_y = 0;
    o->aim_x = 0;
    o->recoil = w->recoil;
    if (o->cool == 0) {
        if (o->queued == 0) {
            func_801C7B1C(o, w, 0, 0);
            if (o->rounds == 0)
                return;
            if (w->chance < (func_8002A4A0() & 0xFF))
                return;
            if ((w->type & 0x20) && o->rounds < w->rounds)
                return;
            o->queued = o->rounds;
            o->shots = 0;
            o->rounds = 0;
        }
        if (w->type & 0x10)
            n = o->queued;
        else
            n = 1;
        kind = w->type & 0xF;
        for (n--; n != -1; n--) {
            o->queued--;
            if ((o->target->flags & 8) && func_8002A4A0() < 16384)
                break;
            if (!(w->type & 0x40)) {
                shot.proj = kind < 10 ? w->proj_a | 0x800 : w->proj_a | 0x900;
                shot.sub = w->sub_a;
            } else if (o->queued & 1) {
                shot.proj = kind < 10 ? w->proj_a | 0x800 : w->proj_a | 0x900;
                shot.sub = w->sub_a;
            } else {
                shot.proj = kind < 10 ? w->proj_b | 0x800 : w->proj_b | 0x900;
                shot.sub = w->sub_b;
            }
            if (!(w->type & 0x80))
                side = 0;
            else if (!(w->type & 0x40))
                side = o->queued << 1;
            else
                side = o->queued & 0xFFFE;
            if (kind < 10) {
                sp = &D_8008E610[w->type & 0xF];
                shot.a = sp->a;
                shot.b = 48;
                shot.c = sp->c;
                dx = func_80014D78(sp->spread_x) - (sp->spread_x >> 1);
                func_80077B78(o, w->b22, w->h24, w->b26, &shot, 1, side, side + 1, dx,
                              func_80014D78(sp->spread_y) - (sp->spread_y >> 1), o->target);
            } else {
                shot.a = w->type - 10;
                func_80077B78(o, w->b22, w->h24, w->b26, &shot, 1, side, side + 1);
            }
        }
        if (o->queued == 0)
            o->cool = w->cool;
    } else {
        o->cool--;
    }
}
