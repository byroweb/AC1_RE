#include "ac1/ac1.h"
/* ovl203: fire the selected weapon w of the side in D_80039D18 (aim pose,
 * trigger edge, muzzle, recoil, launch by weapon kind), then run the cool-down. */
typedef struct {                /* 6 */
    u16 range;
    u16 kick;
    u8 mode;                    /* 0 = edge, 1 = held */
    u8 pad5;
} FSpec3;
typedef struct {
    u8 pad0;
    u8 idx;                     /* +0x01 */
    u8 b2, b3;                  /* +0x02 */
    u8 pad4[6];
    s8 sA;                      /* +0x0A */
    u8 bB;                      /* +0x0B */
    u8 padc[4];
    u16 ammo;                   /* +0x10 */
    u8 pad12[4];
    s16 h16;                    /* +0x16 */
    u8 pad18[0x23 - 0x18];
    u8 b23;                     /* +0x23 */
    u8 x24[4];                  /* +0x24 */
} FWpn3;
typedef struct {
    u8 pad0[0x26];
    u16 flags;
    u8 pad28[0x7C - 0x28];
} FPart3;
typedef struct {
    u8 pad0[0x0E];
    s16 id;                     /* +0x0E */
    u8 pad10[0x20 - 0x10];
    FPart3 *parts;              /* +0x20 */
} FMdl3;
typedef struct {
    u8 pad0[0x3C];
    u16 part;                   /* +0x3C */
} FRec3;
typedef struct {
    u8 pad0[0x14];
    s32 off;                    /* +0x14 */
} FSet3;
typedef struct FObj3 FObj3;
struct FObj3 {
    u8 pad0[0x0E];
    s16 h0E;                    /* +0x0E */
    u8 pad10[0x18 - 0x10];
    s16 h18, h1A, h1C;          /* +0x18 */
    u8 pad1e[0x36 - 0x1E];
    u16 h36;                    /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;                     /* +0x3C */
    u8 kind;                    /* +0x3D */
    u8 pad3e;
    u8 cool;                    /* +0x3F */
    u8 pad40;
    u8 b41;                     /* +0x41 */
    u8 pad42[0x50 - 0x42];
    void (*fn50)(FObj3 *, u8);  /* +0x50 */
    u8 pad54[0x60 - 0x54];
    FMdl3 *mdl;                 /* +0x60 */
    u8 mode;                    /* +0x64 */
    u8 b65;                     /* +0x65 */
    u8 pad66[0x74 - 0x66];
    s16 h74;                    /* +0x74 */
    s16 h76, h78, h7A;          /* +0x76 */
    s16 recoil;                 /* +0x7C */
    u8 pad7e[0xCC - 0x7E];
    s16 vel[3];                 /* +0xCC */
};
typedef struct {
    u8 pad0[8];
    u32 held;                   /* +0x08 */
    u32 prev;                   /* +0x0C */
    u8 pad10[0x1A - 0x10];
    u8 b1A;                     /* +0x1A */
    u8 pad1b[0x84 - 0x1B];
    void *lock;                 /* +0x84 */
    u8 pad88[0x94 - 0x88];
    u8 x94[0x18];               /* +0x94 */
    u8 xAC[4];                  /* +0xAC */
    u8 padb0[0x21C - 0xB0];
    u16 h21C;                   /* +0x21C */
    u8 pad21e[0x27A - 0x21E];
    u8 b27A;                    /* +0x27A */
    u8 pad27b[0x286 - 0x27B];
    u16 h286;                   /* +0x286 */
    u8 pad288[0x290 - 0x288];
    u16 k290, k292;             /* +0x290 */
    u8 pad294[0x29A - 0x294];
    u16 k29A;                   /* +0x29A */
    u16 k29C;                   /* +0x29C */
    u8 pad29e[0x2A0 - 0x29E];
    u16 k2A0;                   /* +0x2A0 */
    u8 pad2a2[2];
    u16 k2A4;                   /* +0x2A4 */
} FSide3;
extern FSide3 *D_80039D18;
extern FSpec3 D_8008A348[];
extern FSet3 *D_8019DB6C[];
extern u8 D_8008A914[], D_8008A918[], D_8008A91C[];
extern void func_80078C30();
extern void func_8007A590(void);
extern s32 func_8004EE14(u8 *mode);
extern void func_8007A5EC(FObj3 *o, FWpn3 *w, s32 range, s32 a, s32 b);
extern s32 func_8007B27C(FWpn3 *w);
extern s32 func_800732FC(FObj3 *o, SVECTOR *v, s32 a, s32 b, s32 c, s32 d, s32 e);
extern void VectorNormalSS(SVECTOR *a, SVECTOR *b);
extern void func_8007FCCC(FObj3 *o, s32 a, s32 b, s32 c, s16 *vel);
extern void func_80084B44(FObj3 *o, u8 *x, SVECTOR *a, SVECTOR *b, s32 r, s32 h, s32 c, u8 *d, s32 e, s32 f, s32 g, u8 *h2, u8 *i, s32 j, s32 k);
extern void func_80087660(FObj3 *o, SVECTOR *a, SVECTOR *b, u8 *x, s32 h, s32 k);
extern void func_800870B8(FObj3 *o, SVECTOR *a, SVECTOR *b, u8 *x, s32 h, s32 k);
extern void func_80087E08(FObj3 *o, SVECTOR *a, SVECTOR *b, u8 *x, s32 h, s32 k, s32 l, SVECTOR *m);
extern s32 func_80015258(SVECTOR *a, s32 b, s32 c, u8 *d, u8 *e, SVECTOR *f, s32 g, s32 h);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);
extern void func_800850F4(FObj3 *o, u8 *x, SVECTOR *a, SVECTOR *b, SVECTOR *c, s32 d, s32 e, s32 f);
extern void func_80088C28(void *t, s32 a, FObj3 *o);
extern void func_80078BF8(FObj3 *o, FWpn3 *w);
extern void func_80078C50(FObj3 *o, FWpn3 *w);
extern void func_80078C6C(FObj3 *o, FWpn3 *w);

void func_8007BE54(FObj3 *o, FWpn3 *w)
{
    SVECTOR v[2];
    SVECTOR ang;
    SVECTOR tmp;
    FSpec3 *spec;
    FMdl3 *m;
    FSet3 *set;
    FRec3 *rec;
    FPart3 *pp;
    s32 k;
    s32 n;
    s32 fire;
    s32 snd;

    spec = &D_8008A348[w->idx];
    o->recoil = -1;
    func_80078C30();
    if (D_80039D18->b27A != 1 && D_80039D18->b27A != 3 && o->kind == 192) {
        func_8007A590();
        o->h76 = o->h78 = o->h7A = 0;
        if (D_80039D18->b27A == 2) {
            o->recoil = w->sA;
        }
        goto cool;
    }
    if (D_80039D18->b27A == 0) {
        if (o->dir != 34) {
            goto arm;
        }
        if (o->mode != D_80039D18->b1A || !func_8004EE14(&o->mode)) {
            goto posed;
        }
    }
    o->recoil = w->sA;
    func_80078C30(o, w);
    m = o->mdl;
    if (m == 0 || m->id != w->sA) {
        goto cool;
    }
    set = D_8019DB6C[o->h0E];
    rec = (FRec3 *)((u8 *)set + ((s32 *)((u8 *)set + set->off))[w->sA]);
    pp = m->parts + rec->part;
    if (pp->flags & 4) {
        goto cool;
    }
    func_8007A5EC(o, w, spec->range, 1, D_80039D18->h286);
    if (!((o->mode == D_80039D18->b1A && func_8004EE14(&o->mode)) || D_80039D18->b27A != 0) || o->b65 != 255) {
        goto tick;
    }
    if (o->cool != 0) {
        goto run;
    }
    if (w->ammo == 0) {
        goto tick;
    }
    if (!((spec->mode == 0 && (D_80039D18->held & D_80039D18->k29C) && !(D_80039D18->prev & D_80039D18->k29C))
          || (spec->mode == 1 && (D_80039D18->held & D_80039D18->k29C)))) {
        goto tick;
    }
    if (!func_8007B27C(w)) {
        goto tick;
    }
    w->ammo--;
    if (func_800732FC(o, v, w->b2, w->b3, 2, 0, 1)) {
        v[1].vx -= v[0].vx;
        v[1].vy -= v[0].vy;
        v[1].vz -= v[0].vz;
        VectorNormalSS(&v[1], &v[1]);
        if (D_80039D18->b27A == 3 && o->kind == 16) {
            fire = 0;
        } else {
            k = (D_80039D18->h21C * spec->kick) >> 8;
            fire = 1;
            if (k != 0) {
                o->vel[0] -= (v[1].vx * k) >> 6;
                o->vel[1] -= (v[1].vy * k) >> 6;
                o->vel[2] -= (v[1].vz * k) >> 6;
                if ((u32)(o->kind - 2) < 2) {
                    func_8007FCCC(o, 0, 0, 256, o->vel);
                }
            }
        }
        switch (w->idx) {
        case 0:
            func_80084B44(o, w->x24, &v[0], &v[1], spec->range, w->h16, 7, D_8008A914, 20, 3, 120,
                          D_8008A918, D_8008A91C, 0, 4);
            func_80088C28(D_80039D18->lock, 1, o);
            if (fire && o->fn50) {
                o->fn50(o, o->b41);
            }
            func_80078BF8(o, w);
            break;
        case 1:
            func_80087660(o, &v[0], &v[1], w->x24, w->h16, 6);
            func_80088C28(D_80039D18->lock, 1, o);
            if (fire && o->fn50) {
                o->fn50(o, o->b41);
            }
            func_80078BF8(o, w);
            break;
        case 2:
            func_80087660(o, &v[0], &v[1], w->x24, w->h16, 7);
            func_80088C28(D_80039D18->lock, 3, o);
            if (fire && o->fn50) {
                o->fn50(o, o->b41);
            }
            func_80078BF8(o, w);
            break;
        case 3:
            v[1].vx = (v[1].vx * 75) >> 9;
            v[1].vy = (v[1].vy * 75) >> 9;
            v[1].vz = (v[1].vz * 75) >> 9;
            if (D_80039D18->lock != 0) {
                n = func_80015258(&v[0], 0, 600, D_80039D18->x94, D_80039D18->xAC, &tmp, 0, 8);
            } else {
                n = (s16)(w->h16 / 600);
            }
            func_80014AE4(v[1].vx, v[1].vy, v[1].vz, &ang);
            func_800850F4(o, w->x24, &v[0], &ang, &v[1], 49, 0, n);
            func_80088C28(D_80039D18->lock, 3, o);
            if (fire && o->fn50) {
                o->fn50(o, o->b41);
            }
            func_80078BF8(o, w);
            break;
        case 4:
            func_80087660(o, &v[0], &v[1], w->x24, w->h16, 5);
            func_80088C28(D_80039D18->lock, 1, o);
            if (fire && o->fn50) {
                o->fn50(o, o->b41);
            }
            func_80078BF8(o, w);
            break;
        case 5:
            func_800870B8(o, &v[0], &v[1], w->x24, w->h16, 1);
            func_80088C28(D_80039D18->lock, 3, o);
            if (fire && o->fn50) {
                o->fn50(o, o->b41);
            }
            func_80078BF8(o, w);
            break;
        case 6:
            func_80014AE4(v[1].vx, v[1].vy, v[1].vz, &ang);
            func_80087E08(o, &v[0], &v[1], w->x24, w->h16, 3, 49, &ang);
            func_80088C28(D_80039D18->lock, 3, o);
            if (fire && o->fn50) {
                o->fn50(o, o->b41);
            }
            func_80078BF8(o, w);
            break;
        }
    }
    snd = w->bB;
    o->h74 = 0;
    o->b65 = snd;
    o->cool = w->b23;
tick:
    if (o->cool != 0) {
        goto run;
    }
    if (D_80039D18->b27A == 0
        && (D_80039D18->held & (D_80039D18->k2A4 | (D_80039D18->k2A0 | (D_80039D18->k292 | (D_80039D18->k29A | D_80039D18->k290)))))) {
        o->dir = 35;
        o->h36 |= 0x10;
    }
    goto cool;
posed:
    if (o->dir == 34) {
        goto cool;
    }
arm:
    D_80039D18->lock = 0;
    if ((D_80039D18->held & D_80039D18->k29C) && !(D_80039D18->prev & D_80039D18->k29C)
        && o->kind != 192 && o->kind != 176) {
        D_80039D18->b1A = w->sA + 42;
        o->h18 = o->h1A = o->h1C = 0;
        o->dir = 34;
        o->h36 |= 0x10;
    }
cool:
    if (o->cool == 0) {
        return;
    }
run:
    if (o->cool >= 20) {
        func_80078C50(o, w);
    } else {
        func_80078C6C(o, w);
    }
    if (o->b65 == 255) {
        o->cool--;
    }
}
