/* ovl202 0x8007D2F8  size 2208  player weapon fire (one shot per trigger): aim, recoil,
 * launch the projectile kind by am->id (0..6), then run the cool-down. Hand rewrite. */
#include "ac1/ac1.h"

typedef struct {
    u16 range;          /* +0x00 */
    u16 kick;           /* +0x02 */
    u8 trig;            /* +0x04 0 = semi, 1 = auto */
    u8 pad5;
} Spec;

typedef struct {
    u8 b0;
    u8 id;              /* +0x01 */
    u8 mz_a0, mz_b0;    /* +0x02 */
    u8 pad4[6];
    u8 recoil;          /* +0x0A (signed) */
    u8 slot;            /* +0x0B */
    u8 padc[4];
    u16 count;          /* +0x10 */
    u8 pad12[4];
    s16 speed;          /* +0x16 */
    u8 pad18[0x0B];
    u8 cooldown;        /* +0x23 */
    u8 fx[4];           /* +0x24 */
} Ammo;

typedef struct {
    u8 pad0[0x26];
    u16 flags;          /* +0x26 */
    u8 pad28[0x7C - 0x28];
} Part;

typedef struct {
    u8 pad0[0x0E];
    s16 id;             /* +0x0E */
    u8 pad10[0x20 - 0x10];
    Part *parts;        /* +0x20 */
} Model;

typedef struct {
    u8 pad0[0x3C];
    u16 part;           /* +0x3C */
} Rec;

typedef struct {
    u8 pad0[0x14];
    s32 table;          /* +0x14 */
} Set;

typedef struct Obj Obj;
struct Obj {
    u8 pad0[0x0E];
    s16 set;            /* +0x0E */
    u8 pad10[0x18 - 0x10];
    s16 h18, h1A, h1C;  /* +0x18 */
    u8 pad1e[0x36 - 0x1E];
    u16 flags;          /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;             /* +0x3C */
    u8 kind;            /* +0x3D */
    u8 pad3e;
    u8 cool;            /* +0x3F */
    u8 pad40;
    u8 b41;             /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn_fire)(Obj *, s32); /* +0x50 */
    u8 pad54[0x60 - 0x54];
    Model *model;       /* +0x60 */
    u8 mover;           /* +0x64 */
    u8 weapon_slot;     /* +0x65 */
    u8 pad66[0x74 - 0x66];
    s16 fire_timer;     /* +0x74 */
    s16 h76, h78, h7A;  /* +0x76 */
    s16 recoil;         /* +0x7C */
    u8 pad7e[0xCC - 0x7E];
    u16 vel[3];         /* +0xCC */
};

extern Spec D_8008E658[];
extern Set *D_801A6134[];
extern s32 D_80040FA8;
extern s32 D_80040FAC;
extern u8 D_80040FBA;
extern u16 D_800411BC;
extern u8 D_8004121A;
extern u16 D_80041226;
extern u16 D_80041230;
extern u16 D_80041232;
extern u16 D_8004123A;
extern u16 D_8004123C;
extern u16 D_80041240;
extern u16 D_80041244;
extern u8 D_8008EC24[];
extern u8 D_8008EC28[];
extern u8 D_8008EC2C[];

extern void func_8007A140(Obj *e, Ammo *am);
extern void func_8007A108(Obj *e, Ammo *am);
extern void func_8007A160(Obj *e, Ammo *am);
extern void func_8007A17C(Obj *e, Ammo *am);
extern void func_8007BA2C(void);
extern void func_8007BA98(Obj *e, Ammo *am, s32 range, s32 aimed, s32 x);
extern s32 func_8007C730(Ammo *am);
extern s32 func_8004EBF8(u8 *mover);
extern s32 func_800745D0(Obj *e, SVECTOR *muzzle, s32 a, s32 b, s32 c, s32 d, s32 f);
extern void func_80081088(Obj *e, s32 a, s32 b, s32 c, u16 *vel);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);
extern s32 func_80015258(SVECTOR *a, s32 b, s32 c, s16 *d, u8 *f, SVECTOR *g, s32 h, s32 i);
extern void func_80085E98(Obj *e, u8 *fx, SVECTOR *m, SVECTOR *d, s32 range, s32 speed, s32 a, u8 *b, s32 c, s32 dd, s32 z, u8 *f, u8 *g, s32 h, s32 i);
extern void func_800889B4(Obj *e, SVECTOR *m, SVECTOR *d, u8 *fx, s32 speed, s32 a);
extern void func_8008840C(Obj *e, SVECTOR *m, SVECTOR *d, u8 *fx, s32 speed, s32 a);
extern void func_8008915C(Obj *e, SVECTOR *m, SVECTOR *d, u8 *fx, s32 speed, s32 a, s32 b, SVECTOR *f);
extern void func_80086448(Obj *e, u8 *fx, SVECTOR *m, SVECTOR *ang, SVECTOR *d, s32 a, s32 b, s32 speed);
extern void func_80089F7C(AcEntity *t, s32 a, Obj *e);

void func_8007D2F8(Obj *e, Ammo *am)
{
    SVECTOR m;
    SVECTOR d;
    SVECTOR ang;
    SVECTOR out;
    Spec *spec;
    Model *mdl;
    Set *set;
    Rec *rec;
    Part *pp;
    u8 *mode;
    s32 k;
    s32 notify;
    s32 speed;
    s32 kick;

    spec = &D_8008E658[am->id];
    e->recoil = -1;
    func_8007A140(e, am);
    mode = &D_8004121A;
    if (*mode != 1 && *mode != 3 && e->kind == 0xC0) {
        func_8007BA2C();
        e->h7A = 0;
        e->h78 = 0;
        e->h76 = 0;
        if (*mode == 2) {
            e->recoil = (s8)am->recoil;
        }
    } else if (*mode != 0
               || (e->dir == 0x22 && e->mover == D_80040FBA && func_8004EBF8(&e->mover))) {
        e->recoil = (s8)am->recoil;
        func_8007A140(e, am);
        mdl = e->model;
        if (mdl != 0 && mdl->id == (s8)am->recoil) {
            set = D_801A6134[e->set];
            rec = (Rec *)((u8 *)set + ((s32 *)((u8 *)set + set->table))[(s8)am->recoil]);
            pp = mdl->parts + rec->part;
            if (!(pp->flags & 4)) {
                func_8007BA98(e, am, spec->range, 1, D_80041226);
                if (((e->mover == D_80040FBA && func_8004EBF8(&e->mover)) || *mode != 0)
                    && e->weapon_slot == 0xFF && e->cool == 0) {
                    if (am->count != 0
                        && ((spec->trig == 0 && (D_80040FA8 & D_8004123C) && !(D_80040FAC & D_8004123C))
                            || (spec->trig == 1 && (D_80040FA8 & D_8004123C)))
                        && func_8007C730(am) != 0) {
                        am->count--;
                        if (func_800745D0(e, &m, am->mz_a0, am->mz_b0, 2, 0, 1) != 0) {
                            d.vx -= m.vx;
                            d.vy -= m.vy;
                            d.vz -= m.vz;
                            VectorNormalSS(&d, &d);
                            if (D_8004121A == 3 && e->kind == 0x10) {
                                notify = 0;
                            } else {
                                kick = (D_800411BC * spec->kick) >> 8;
                                notify = 1;
                                if (kick != 0) {
                                    e->vel[0] -= (d.vx * kick) >> 6;
                                    e->vel[1] -= (d.vy * kick) >> 6;
                                    e->vel[2] -= (d.vz * kick) >> 6;
                                    if (e->kind == 2 || e->kind == 3) {
                                        func_80081088(e, 0, 0, 256, e->vel);
                                    }
                                }
                            }
                            switch (am->id) {
                            case 0:
                                func_80085E98(e, am->fx, &m, &d, spec->range, am->speed, 7, D_8008EC24,
                                              20, 3, 120, D_8008EC28, D_8008EC2C, 0, 4);
                                func_80089F7C(g_target_list[0].target, 1, e);
                                break;
                            case 1:
                                func_800889B4(e, &m, &d, am->fx, am->speed, 6);
                                func_80089F7C(g_target_list[0].target, 1, e);
                                break;
                            case 2:
                                func_800889B4(e, &m, &d, am->fx, am->speed, 7);
                                func_80089F7C(g_target_list[0].target, 3, e);
                                break;
                            case 3:
                                d.vx = (d.vx * 75) >> 9;
                                d.vy = (d.vy * 75) >> 9;
                                d.vz = (d.vz * 75) >> 9;
                                if (g_target_list[0].target != 0) {
                                    speed = func_80015258(&m, 0, 600, g_target_list[0].proj,
                                                          &g_target_list[0].unk20[8], &out, 0, 8);
                                } else {
                                    speed = (s16)(am->speed / 600);
                                }
                                func_80014AE4(d.vx, d.vy, d.vz, &ang);
                                func_80086448(e, am->fx, &m, &ang, &d, 49, 0, speed);
                                func_80089F7C(g_target_list[0].target, 3, e);
                                break;
                            case 4:
                                func_800889B4(e, &m, &d, am->fx, am->speed, 5);
                                func_80089F7C(g_target_list[0].target, 1, e);
                                break;
                            case 5:
                                func_8008840C(e, &m, &d, am->fx, am->speed, 1);
                                func_80089F7C(g_target_list[0].target, 3, e);
                                break;
                            case 6:
                                func_80014AE4(d.vx, d.vy, d.vz, &ang);
                                func_8008915C(e, &m, &d, am->fx, am->speed, 3, 49, &ang);
                                func_80089F7C(g_target_list[0].target, 3, e);
                                break;
                            default:
                                goto done;
                            }
                            if (notify != 0 && e->fn_fire != 0) {
                                e->fn_fire(e, e->b41);
                            }
                            func_8007A108(e, am);
                        }
                    done:
                        e->weapon_slot = am->slot;
                        e->fire_timer = 0;
                        e->cool = am->cooldown;
                    }
                }
                if (e->cool == 0 && D_8004121A == 0
                    && (D_80040FA8 & (D_80041244 | (D_80041240 | (D_80041232 | (D_8004123A | D_80041230)))))) {
                    e->dir = 0x23;
                    e->flags |= 0x10;
                }
            }
        }
    } else if (e->dir != 0x22) {
        g_target_list[0].target = 0;
        if ((D_80040FA8 & D_8004123C) && !(D_80040FAC & D_8004123C)
            && e->kind != 0xC0 && e->kind != 0xB0) {
            D_80040FBA = am->recoil + 42;
            e->h1C = 0;
            e->h1A = 0;
            e->h18 = 0;
            e->dir = 0x22;
            e->flags |= 0x10;
        }
    }
    if (e->cool != 0) {
        if (e->cool >= 20) {
            func_8007A160(e, am);
        } else {
            func_8007A17C(e, am);
        }
        if (e->weapon_slot == 0xFF) {
            e->cool--;
        }
    }
}
