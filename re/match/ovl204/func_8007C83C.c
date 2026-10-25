#include "ac1/ac1.h"
/* MATCH (wave6 porter2): ovl204 copy of matched ovl203 0x8007CB90 (symbol renames only). Needs --rodata 0x8004B180. */
/* ovl203 per-frame AC control for the side in D_80039D18: wreck sparks when
 * dead, else incoming-missile intercept, energy/boost bookkeeping, weapon
 * switch and fire, and the dash (func_80072AD4) on its button. */
typedef struct {
    u8 pad0[8];
    s16 px, py, pz;             /* +0x08 */
    u8 pade[2];
    s16 h0E_dummy;
} CObjHead3;
typedef struct {
    u8 pad0[8];
    s16 px;                     /* +0x08 */
    s16 py;                     /* +0x0A */
    s16 pz;                     /* +0x0C */
    s16 h0E;                    /* +0x0E */
    u8 x10[2];                  /* +0x10 */
    s16 h12;                    /* +0x12 */
    u8 pad14[0x36 - 0x14];
    u16 h36;                    /* +0x36 */
    u8 pad38[0x3C - 0x38];
    u8 dir;                     /* +0x3C */
    u8 kind;                    /* +0x3D */
    u8 pad3e;
    u8 cool;                    /* +0x3F */
    u8 b40;                     /* +0x40 */
    u8 pad41[0x60 - 0x41];
    void *mdl;                  /* +0x60 */
    u8 pad64;
    u8 b65;                     /* +0x65 */
    u8 pad66[0x6C - 0x66];
    u16 h6C;                    /* +0x6C */
    u8 pad6e[0x7C - 0x6E];
    s16 recoil;                 /* +0x7C */
    u8 pad7e[0xCC - 0x7E];
    s16 vel[3];                 /* +0xCC */
    u8 padd2[0x110 - 0xD2];
    s32 w110;                   /* +0x110 */
    u8 pad114[0x148 - 0x114];
    u16 h148;                   /* +0x148 */
    u8 pad14a[0x15C - 0x14A];
    u32 w15C;                   /* +0x15C */
} CObj3;
typedef struct {
    s16 pos[3];                 /* +0x00 */
    s16 free;                   /* +0x06 */
    void *owner;                /* +0x08 */
    u8 padc[6];
    s16 h12;                    /* +0x12 */
    void (*think)();            /* +0x14 */
    u8 pad18[0xE0 - 0x18];
} CProj3;
typedef struct {
    void *t;
    u8 pad4[0x10 - 4];
    SVECTOR v10;                /* +0x10 (pad +0x16 = flags) */
} CHit3;
typedef struct {
    u8 pad0[8];
    u32 held;                   /* +0x08 */
    u32 prev;                   /* +0x0C */
    u8 pad10[0x18 - 0x10];
    u8 cur;                     /* +0x18 */
    u8 sel;                     /* +0x19 */
    u8 pad1a[0x1E - 0x1A];
    u16 en;                     /* +0x1E */
    u16 h20;                    /* +0x20 */
    u8 pad22[0x84 - 0x22];
    void *lock;                 /* +0x84 */
    u8 pad88[0x94 - 0x88];
    SVECTOR lockv;              /* +0x94 */
    u8 pad9c[0x220 - 0x9C];
    s16 yaw;                    /* +0x220 */
    u8 pad222[0x254 - 0x222];
    u32 flags;                  /* +0x254 */
    u8 pad258[0x262 - 0x258];
    u8 icept;                   /* +0x262 */
    u8 icone;                   /* +0x263 */
    u16 en_max;                 /* +0x264 */
    u16 h266;                   /* +0x266 */
    u16 h268;                   /* +0x268 */
    u16 h26A;                   /* +0x26A */
    u16 h26C;                   /* +0x26C */
    u16 dash_cost;              /* +0x26E */
    u8 pad270[0x27A - 0x270];
    u8 b27A;                    /* +0x27A */
    u8 pad27b[0x298 - 0x27B];
    u16 k298;                   /* +0x298 */
    u16 k29A;                   /* +0x29A */
    u16 k29C;                   /* +0x29C */
    u16 k29E;                   /* +0x29E */
    u8 pad2a0[0x2A8 - 0x2A0];
    u8 b2A8, b2A9;              /* +0x2A8 */
    u8 pad2aa[0x2BA - 0x2AA];
    u16 h2BA;                   /* +0x2BA */
    u16 h2BC;
    u16 h2BE;                   /* +0x2BE */
    u8 pad2c0[0x2D8 - 0x2C0];
    u8 nweap;                   /* +0x2D8 */
    u8 pad2d9;
    u8 weap[3][0x3C];           /* +0x2DA */
} CSide3;
extern CSide3 *D_80039D18;
extern u32 D_801AB684;
extern CProj3 D_801B645C[256];
extern u8 D_8008A74C[], D_8008A770[], D_8008A778[], D_8008A77C[];
extern u8 D_801AB68E[];
extern u8 D_801AB748[][0x170];
extern void func_80082D70();
extern s32 func_80072F08();
extern void func_8007712C(void);
extern s32 func_800554F8(void *m, s32 a, void *pos, void *b, s32 n, SVECTOR *out);
extern s16 *func_8008597C(s32 a, s16 *p, s32 b, s32 c, s32 d, u8 *e, u8 *f, u8 *g, u8 *h);
extern s32 func_80074094(CObj3 *o, u8 *a);
extern s32 rand(void);
extern s32 func_80073138(CObj3 *o, SVECTOR *v, s32 a, s32 b, s32 c, s32 d);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);
extern s16 *func_80084470(s32 a, SVECTOR *p, void *t, s32 b, void *c, s32 d);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern void func_8004D0C8(s32 id, s32 vol);
extern void func_8007A240(void);
extern void func_8007C7B4();
extern void func_8007AF8C(CObj3 *o);
extern void func_8007AFC8(CObj3 *o, u8 *w);
extern void func_8007B874(CObj3 *o, u8 *w);
extern void func_8007BB04(CObj3 *o, u8 *w);
extern s32 func_80088FBC(void *pos, void *b);
extern s32 func_80072AD4(SVECTOR *p, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, s32 h, s32 i, u8 *buf);
extern CHit3 *func_80015440(u8 *buf, s32 n, s32 sz, s32 (*cmp)(), s32 x);

typedef struct { u8 r, g, b, cd; } Col3;
static const Col3 D_8004B180 = {250, 50, 0, 0};

void func_8007C83C(CObj3 *o)
{
    SVECTOR v[12];
    u8 buf[96];

    func_8007712C();
    if (o->b40 != 255) {
        s16 *p;
        s32 i;

        D_80039D18->flags &= ~0x10000;
        o->h148 = 0;
        o->w15C = 0;
        if (func_800554F8(o->mdl, o->h0E + 128, &o->px, o->x10, 2, v)) {
            i = 2;
            p = (s16 *)v;
            do {
                i--;
                func_8008597C(0, p + 2, 41, 20, (rand() & 1) + 3, D_8008A770, D_801AB68E, D_8008A778, D_8008A77C)[9] = 80;
                p += 16;
            } while (i != 0);
        }
        if (o->b40 != 254 && func_80074094(o, D_8008A74C)) {
            o->b40 = 254;
        }
    } else {
        CProj3 *best;
        s32 bestd;
        s32 a1;
        s32 a2;
        s32 n;
        u8 *w;
        u16 en;
        s32 m;

        if (!(D_801AB684 & 0x400)) {
            if ((rand() & 0x7F) < D_80039D18->icept || (o->w110 & 1)) {
                best = 0;
                bestd = 0x7FFFFFFF;
                if (func_80073138(o, v, D_80039D18->b2A8, D_80039D18->b2A9, 1, 0)) {
                    CProj3 *q;
                    CProj3 *pr;
                    s32 k;
                    s32 cone;
                    s32 ncone;
                    s32 mask;
                    u32 me;

                    q = D_801B645C;
                    k = 256;
                    mask = 0x7FFFFFFF;
                    me = (u32)o & bestd;
                    cone = D_80039D18->icone * 8;
                    ncone = -cone;
                    pr = q;
                    do {
                        if (pr->free == 0 && pr->think == func_80082D70 && pr->h12 > 0
                            && ((u32)pr->owner & mask) != me) {
                            s32 dx, dy, dz, d, ang;

                            dx = q->pos[0] - v[0].vx;
                            dy = pr->pos[1] - v[0].vy;
                            dz = pr->pos[2] - v[0].vz;
                            if ((u32)(dx + 5000) < 10001 && (u32)(dy + 5000) < 10001 && (u32)(dz + 5000) < 10001) {
                                d = dx * dx + dz * dz;
                                if (d <= 25000000 && d < bestd) {
                                    func_80014AE4(dx, dy, dz, &v[3]);
                                    ang = (v[3].vx - D_80039D18->yaw) & 0xFFF;
                                    if (ang >= 2048) {
                                        ang -= 4096;
                                    }
                                    if (ang >= ncone && cone >= ang) {
                                        ang = (v[3].vy - o->h12) & 0xFFF;
                                        if (ang >= 2048) {
                                            ang -= 4096;
                                        }
                                        if (ang >= ncone && cone >= ang) {
                                            bestd = d;
                                            v[2] = v[3];
                                            best = pr;
                                        }
                                    }
                                }
                            }
                        }
                        pr++;
                        k--;
                        q++;
                    } while (k != 0);
                }
                if (best != 0) {
                    *(Col3 *)&v[3] = D_8004B180;
                    if (o->w110 == 0) {
                        o->w110 = 15;
                    }
                    if (rand() < 2048) {
                        func_80084470(0, v, best, 4, &v[3], 10)[9] = 0;
                        best->h12 = 0;
                    } else {
                        v[2].vx = (rand() & 0x1F) - (s16)(v[2].vx + 16);
                        v[2].vy += (rand() & 0x1F) - 16;
                        func_80014774(&v[2], &v[4]);
                        v[4].vx = v[0].vx + ((v[4].vx * 625) >> 9);
                        v[4].vy = v[0].vy + ((v[4].vy * 625) >> 9);
                        v[4].vz = v[0].vz + ((v[4].vz * 625) >> 9);
                        func_80084470(0, v, &v[4], 4, &v[3], 10)[9] = 0;
                    }
                }
                o->w110--;
            }
        }
        a2 = D_80039D18->h266;
        switch (o->kind) {
        case 0:
        case 1:
            a1 = D_80039D18->h268;
            break;
        default:
            if (D_80039D18->b27A == 0 || D_80039D18->b27A == 3) {
                if (!(o->kind == 16 || o->kind == 176)) {
                    a1 = D_80039D18->h268;
                } else {
                    a1 = D_80039D18->h26C;
                }
            } else if (!(o->kind == 16 || o->kind == 176)) {
                a1 = D_80039D18->h268;
            } else {
                if (o->kind != 16 && (o->vel[0] != 0 || o->vel[2] != 0)) {
                    a1 = D_80039D18->h26C;
                } else {
                    a1 = D_80039D18->h268;
                }
            }
            break;
        }
        if (D_80039D18->h20 & 1) {
            a1 >>= 1;
            goto recharge;
        }
        en = D_80039D18->en;
        if (en < D_80039D18->h26A) {
            a1 >>= 2;
            a2 >>= 2;
        }
        if ((D_80039D18->held & D_80039D18->k29A) && o->dir != 129 && (o->kind == 192 || o->kind == 176)) {
            if (o->kind == 176) {
                a2 >>= 1;
            }
            if (a2 < en) {
                D_80039D18->en = en - a2;
            } else {
                D_80039D18->en = 0;
                D_80039D18->h20 |= 1;
            }
} else {
        recharge:
            if (D_80039D18->en < D_80039D18->en_max) {
                D_80039D18->en += a1;
            } else {
                D_80039D18->en = D_80039D18->en_max;
                D_80039D18->h20 &= ~1;
            }
}
        if (D_80039D18->nweap != 0 && (D_80039D18->held & D_80039D18->k298) && !(D_80039D18->prev & D_80039D18->k298)) {
            m = D_80039D18->nweap;
        again:
            D_80039D18->sel++;
            if (D_80039D18->sel >= D_80039D18->nweap) {
                D_80039D18->sel = 0;
            }
            m--;
            if (*(u16 *)(D_80039D18->weap[D_80039D18->sel] + 0x10) == 0 && m != 0)
                goto again;
            func_8004D0C8(45, 100);
            func_8007A240();
        }
        if (o->cool == 0 && D_80039D18->cur != D_80039D18->sel) {
            D_80039D18->cur = D_80039D18->sel;
            o->w15C &= D_80039D18->h2BE;
            if (o->dir == 34 && D_80039D18->b27A == 0) {
                o->dir = 35;
                o->h36 |= 0x10;
            }
            func_8007C7B4();
            func_8007A240();
        }
        if (o->b65 != 255) {
            func_8007AF8C(o);
        }
        o->recoil = -1;
        if ((u32)(o->dir - 32) < 2) {
            return;
        }
        if (D_80039D18->nweap != 0) {
            if (D_801AB684 & 0x400) {
                goto nofire;
            }
            w = D_80039D18->weap[D_80039D18->cur];
            switch (w[0]) {
            case 0:
                func_8007AFC8(o, w);
                break;
            case 1:
                func_8007B874(o, w);
                break;
            case 2:
                func_8007BB04(o, w);
                break;
            }
        } else if (D_801AB684 & 0x400) {
        nofire:
            o->h148 = 0;
            o->w15C = 0;
            if (o->dir == 34 && D_80039D18->b27A == 0) {
                o->dir = 35;
                o->h36 |= 0x10;
            }
        }
        if ((D_80039D18->held & D_80039D18->k29E) && !(D_80039D18->prev & D_80039D18->k29E)
            && !func_80088FBC(&o->px, o->x10) && D_80039D18->h2BA != 0 && !(D_80039D18->h20 & 1)
            && !(o->h36 & 0x10) && D_80039D18->en > D_80039D18->dash_cost) {
            CHit3 *h;

            D_80039D18->en -= D_80039D18->dash_cost;
            v[1].vx = o->px;
            v[1].vz = o->pz;
            v[1].vy = o->py - (o->h6C >> 1);
            n = func_80072AD4(&v[1], o->h12, D_80039D18->yaw, 0, o->h36, 3000, -512, 512, -512, 512, buf);
            func_8007A240();
            D_80039D18->lock = 0;
            if (n != 0) {
                h = func_80015440(buf, n, 48, func_80072F08, 0);
                if ((h->v10.pad & 2) && (u8 *)h->t >= D_801AB748[0] && (u8 *)h->t <= D_801AB748[1]) {
                    D_80039D18->lock = h->t;
                    D_80039D18->lockv = h->v10;
                }
            }
            if (o->kind == 192) {
                o->dir = 33;
                o->h36 |= 0x10;
            } else {
                o->dir = 32;
                o->h36 |= 0x10;
            }
        }
    }
}
