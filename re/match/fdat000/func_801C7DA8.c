/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins): fire a weapon of type 2
 * (801C8CBC dispatches here).  Aim (801C7B1C) with the range from the 6-byte table at
 * 0x8008E658 (indexed by the low nibble of w->type).  Units without the def flag only
 * fire from the attack pose (mode 34); otherwise they switch into it.  A loaded part
 * whose flag 4 is set blocks the shot.  When the cool-down is out and a round is queued
 * (or the chance roll queues the clip), take the muzzle (func_800745D0), recoil the
 * unit along the shot, and launch one of ten projectile kinds by w->type & 0xF.
 * Each muzzle arm makes its own func_800745D0 call and each case its own fire-callback
 * call: cross-jumping folds them into the copies retail keeps. */
#include "ac1/ac1.h"
typedef struct {
    u16 range;          /* +0x00 */
    u16 kick;           /* +0x02 */
    u8 pad4;
    u8 cool;            /* +0x05 */
} Func801C7DA8Spec;
typedef struct {
    u8 pad0[2];
    u16 type;           /* +0x02 */
    u8 part;            /* +0x04 */
    u8 proj_a;          /* +0x05 */
    u8 sub_a;           /* +0x06 */
    u8 proj_b;          /* +0x07 */
    u8 sub_b;           /* +0x08 */
    u8 pad9[2];
    s8 recoil;          /* +0x0B */
    u8 rest;            /* +0x0C */
    u8 padd[2];
    u8 chance;          /* +0x0F */
    u8 clip;            /* +0x10 */
    u8 pad11;
    s16 speed;          /* +0x12 */
    u8 pad14[0x22 - 0x14];
    u8 fx[1];           /* +0x22 */
} Func801C7DA8Weapon;
typedef struct {
    u8 b0;              /* +0x00 */
    u8 pad1[0x24 - 0x01];
    u16 kick;           /* +0x24 */
} Func801C7DA8Def;
typedef struct {
    u8 pad0[0x26];
    u16 flags;          /* +0x26 */
    u8 pad28[0x7C - 0x28];
} Func801C7DA8Part;
typedef struct {
    u8 pad0[0x0E];
    s16 id;             /* +0x0E */
    u8 pad10[0x20 - 0x10];
    Func801C7DA8Part *parts; /* +0x20 */
} Func801C7DA8Model;
typedef struct {
    u16 part;
    u8 pad2[18];
} Func801C7DA8Slot;
typedef struct {
    u8 pad0[0x14];
    Func801C7DA8Slot slots[1]; /* +0x14 */
} Func801C7DA8Rec;
typedef struct {
    u8 pad0[0x14];
    s32 table;          /* +0x14 offset of the s32 record offsets */
} Func801C7DA8Set;
typedef struct Func801C7DA8Obj Func801C7DA8Obj;
struct Func801C7DA8Obj {
    u8 pad0[8];
    AcPos pos;          /* +0x08 pos.w: model set index */
    u8 pad10[0x36 - 0x10];
    u16 faction;        /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 queued;          /* +0x3E */
    u8 cool;            /* +0x3F */
    u8 pad40;
    u8 b41;             /* +0x41 */
    u8 pad42[0x48 - 0x42];
    Func801C7DA8Obj *target; /* +0x48 */
    u8 pad4c[0x50 - 0x4C];
    void (*fn_fire)(Func801C7DA8Obj *, s32); /* +0x50 */
    u8 pad54[0x60 - 0x54];
    Func801C7DA8Model *model; /* +0x60 */
    u8 mode;            /* +0x64 */
    u8 weapon_slot;     /* +0x65 */
    u8 pad66[0x7C - 0x66];
    s16 recoil;         /* +0x7C */
    u8 pad7e[0xCC - 0x7E];
    s16 vel[3];         /* +0xCC */
    u8 padd2[0x108 - 0xD2];
    Func801C7DA8Def *def; /* +0x108 */
    u8 pad10c[0x133 - 0x10C];
    u8 shots;           /* +0x133 */
    u8 pad134;
    u8 pose;            /* +0x135 */
};
extern Func801C7DA8Spec D_8008E658[];
extern Func801C7DA8Set *D_801A6134[];
extern u8 D_801CBB64[];
extern u8 D_801CBB68[];
extern u8 D_801CBB6C[];
extern u8 D_801CBB70[];
extern u8 D_801CBB74[];
extern u8 D_801CBB78[];
extern s32 func_801C7B1C(Func801C7DA8Obj *o, Func801C7DA8Weapon *w, s32 flags, s32 range);
extern s32 func_8004EBF8(u8 *mode);
extern s32 func_8002A4A0(void);
extern s32 func_800745D0(Func801C7DA8Obj *o, SVECTOR *muzzle, s32 a, s32 b, s32 c, s32 d, s32 e);
extern void func_801C5834(Func801C7DA8Obj *o, s32 a, s32 b, s32 c, s16 *vel);
extern s32 func_800781E8(Func801C7DA8Obj *o);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);
extern void func_80085E98();
extern void func_800889B4();
extern void func_8008840C();
extern void func_8008915C();
extern void func_80086448();
void func_801C7DA8(Func801C7DA8Obj *o, Func801C7DA8Weapon *w)
{
    SVECTOR at;
    SVECTOR dir;
    VECTOR ang;
    Func801C7DA8Spec *spec;
    Func801C7DA8Def *def;
    Func801C7DA8Model *m;
    Func801C7DA8Set *set;
    Func801C7DA8Rec *rec;
    Func801C7DA8Part *pp;
    s32 hit, k, dx, dy, dz, speed, r;

    spec = &D_8008E658[w->type & 0xF];
    def = o->def;
    hit = func_801C7B1C(o, w, 0, spec->range);
    if (def->b0 != 0 || (o->dir == 34 && o->mode == o->pose && func_8004EBF8(&o->mode))) {
        o->recoil = w->recoil;
        m = o->model;
        if (m != 0 && m->id == w->recoil) {
            set = D_801A6134[o->pos.w];
            rec = (Func801C7DA8Rec *)((u8 *)set + ((s32 *)((u8 *)set + set->table))[w->recoil]);
            pp = m->parts + (rec->slots + w->part)->part;
            if (pp->flags & 4)
                goto out;
        }
        if (o->cool == 0) {
            if (o->queued == 0) {
                if (hit == 0)
                    goto out;
                if (w->chance < (func_8002A4A0() & 0xFF))
                    goto out;
                o->queued = w->clip;
            }
            if (!(w->type & 0x40)) {
                r = func_800745D0(o, &at, w->proj_a, w->sub_a, 2, 0, 1);
            } else if (o->queued & 1) {
                r = func_800745D0(o, &at, w->proj_a, w->sub_a, 2, 0, 1);
            } else {
                r = func_800745D0(o, &at, w->proj_b, w->sub_b, 2, 0, 1);
            }
            if (r) {
                dir.vx -= at.vx;
                dir.vy -= at.vy;
                dir.vz -= at.vz;
                VectorNormalSS(&dir, &dir);
                k = (o->def->kick * spec->kick) >> 8;
                if (k != 0) {
                    o->vel[0] -= (dir.vx * k) >> 6;
                    o->vel[1] -= (dir.vy * k) >> 6;
                    o->vel[2] -= (dir.vz * k) >> 6;
                    if (o->kind == 2 || o->kind == 3)
                        func_801C5834(o, 0, 0, 256, o->vel);
                }
                switch (w->type & 0xF) {
                case 0:
                    func_80085E98(o, w->fx, &at, &dir, spec->range, w->speed, 7, D_801CBB64, 20, 3,
                                  120, D_801CBB68, D_801CBB6C, 0, 4);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                case 1:
                    func_800889B4(o, &at, &dir, w->fx, w->speed, 7);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                case 2:
                    func_800889B4(o, &at, &dir, w->fx, w->speed, 6);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                case 3:
                    if (!func_800781E8(o))
                        break;
                    dir.vx = (dir.vx * 75) >> 9;
                    dir.vy = (dir.vy * 75) >> 9;
                    dir.vz = (dir.vz * 75) >> 9;
                    if (o->target != 0) {
                        dx = (o->target->pos.x - o->pos.x) >> 1;
                        dy = (o->target->pos.y - o->pos.y) >> 1;
                        dz = (o->target->pos.z - o->pos.z) >> 1;
                        speed = (SquareRoot0(dx * dx + dy * dy + dz * dz) << 2) / 600;
                    } else {
                        speed = (s16)(w->speed / 600);
                    }
                    func_80014AE4(dir.vx, dir.vy, dir.vz, &ang);
                    func_80086448(o, w->fx, &at, &ang, &dir, 49, 0, speed);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                case 4:
                    func_800889B4(o, &at, &dir, w->fx, w->speed, 5);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                case 5:
                    func_8008840C(o, &at, &dir, w->fx, w->speed, 1);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                case 6:
                    func_80014AE4(dir.vx, dir.vy, dir.vz, &ang);
                    func_8008915C(o, &at, &dir, w->fx, w->speed, 3, 49, &ang);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                case 7:
                    func_80014AE4(dir.vx, dir.vy, dir.vz, &ang);
                    func_8008915C(o, &at, &dir, w->fx, w->speed, 1, 0, &ang);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                case 8:
                    func_80014AE4(dir.vx, dir.vy, dir.vz, &ang);
                    func_8008915C(o, &at, &dir, w->fx, w->speed, 2, 0, &ang);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                case 9:
                    func_80085E98(o, w->fx, &at, &dir, spec->range, w->speed, 7, D_801CBB70, 20, 3,
                                  120, D_801CBB74, D_801CBB78, 0, 4);
                    if (o->fn_fire != 0)
                        o->fn_fire(o, o->b41);
                    break;
                }
            }
            o->queued--;
            o->weapon_slot = w->rest;
            o->cool = spec->cool;
        }
    } else if (hit != 0 && o->kind != 192 && o->dir != 34) {
        o->pose = w->recoil + 42;
        o->dir = 34;
        o->shots = 0;
        o->faction |= 0x10;
    }
out:
    if (o->cool != 0)
        o->cool--;
}
