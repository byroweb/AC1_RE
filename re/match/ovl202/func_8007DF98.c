#include "ac1/ac1.h"
/* MATCH (wave6 a3): port of the ovl203 0x8007CB90 wave4 draft; side struct is the global D_80040FA0 (D_80039D18-> in ovl203), buf[768]. Levers: switch case 0/1 before default (2 jumps to default stop CSE keeping o->kind), weapon-switch loop as goto (loop.c would hoist la &sel), spark p = v after i = 2 inside the if. Needs --rodata 0x8004B554. */
/* ovl203 per-frame AC control for the side in D_80039D18: wreck sparks when
 * dead, else incoming-missile intercept, energy/boost bookkeeping, weapon
 * switch and fire, and the dash (func_80073F38) on its button. */
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
extern CSide3 D_80040FA0;
extern u32 D_8019F524;
extern CProj3 D_801AEB3C[256];
extern u8 D_8008EC30[], D_8008EC54[], D_8008EC5C[], D_8008EC60[];
extern u8 D_8019F52E[];
extern u8 D_801A26B8[][0x170];
extern void func_80084314();
extern s32 func_800743A0();
extern void func_80078598(void);
extern s32 func_800564AC(void *m, s32 a, void *pos, void *b, s32 n, SVECTOR *out);
extern s16 *func_80086F20(s32 a, s16 *p, s32 b, s32 c, s32 d, u8 *e, u8 *f, u8 *g, u8 *h);
extern s32 func_800754D0(CObj3 *o, u8 *a);
extern s32 rand(void);
extern s32 func_800745D0(CObj3 *o, SVECTOR *v, s32 a, s32 b, s32 c, s32 d);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);
extern s16 *func_80085A14(s32 a, SVECTOR *p, void *t, s32 b, void *c, s32 d);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern void func_8004CFF4(s32 id, s32 vol);
extern void func_8007BA2C(void);
extern void func_8007DF08();
extern void func_8007C790(CObj3 *o);
extern void func_8007C7C8(CObj3 *o, u8 *w);
extern void func_8007D068(CObj3 *o, u8 *w);
extern void func_8007D2F8(CObj3 *o, u8 *w);
extern s32 func_8008A5CC(void *pos, void *b);
extern s32 func_80073F38(SVECTOR *p, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, s32 h, s32 i, u8 *buf);
extern CHit3 *func_80015440(u8 *buf, s32 n, s32 sz, s32 (*cmp)(), s32 x);

typedef struct { u8 r, g, b, cd; } Col3;
static const Col3 D_8004B1D8 = {250, 50, 0, 0};

void func_8007DF98(CObj3 *o)
{
    SVECTOR v[12];
    u8 buf[768];

    func_80078598();
    if (o->b40 != 255) {
        s16 *p;
        s32 i;

        D_80040FA0.flags &= ~0x10000;
        o->h148 = 0;
        o->w15C = 0;
        if (func_800564AC(o->mdl, o->h0E + 128, &o->px, o->x10, 2, v)) {
            i = 2;
            p = (s16 *)v;
            do {
                i--;
                func_80086F20(0, p + 2, 41, 20, (rand() & 1) + 3, D_8008EC54, D_8019F52E, D_8008EC5C, D_8008EC60)[9] = 80;
                p += 16;
            } while (i != 0);
        }
        if (o->b40 != 254 && func_800754D0(o, D_8008EC30)) {
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

        if (!(D_8019F524 & 0x400)) {
            if ((rand() & 0x7F) < D_80040FA0.icept || (o->w110 & 1)) {
                best = 0;
                bestd = 0x7FFFFFFF;
                if (func_800745D0(o, v, D_80040FA0.b2A8, D_80040FA0.b2A9, 1, 0)) {
                    CProj3 *q;
                    CProj3 *pr;
                    s32 k;
                    s32 cone;
                    s32 ncone;
                    s32 mask;
                    u32 me;

                    q = D_801AEB3C;
                    k = 256;
                    mask = 0x7FFFFFFF;
                    me = (u32)o & bestd;
                    cone = D_80040FA0.icone * 8;
                    ncone = -cone;
                    pr = q;
                    do {
                        if (pr->free == 0 && pr->think == func_80084314 && pr->h12 > 0
                            && ((u32)pr->owner & mask) != me) {
                            s32 dx, dy, dz, d, ang;

                            dx = q->pos[0] - v[0].vx;
                            dy = pr->pos[1] - v[0].vy;
                            dz = pr->pos[2] - v[0].vz;
                            if ((u32)(dx + 5000) < 10001 && (u32)(dy + 5000) < 10001 && (u32)(dz + 5000) < 10001) {
                                d = dx * dx + dz * dz;
                                if (d <= 25000000 && d < bestd) {
                                    func_80014AE4(dx, dy, dz, &v[3]);
                                    ang = (v[3].vx - D_80040FA0.yaw) & 0xFFF;
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
                    *(Col3 *)&v[3] = D_8004B1D8;
                    if (o->w110 == 0) {
                        o->w110 = 15;
                    }
                    if (rand() < 2048) {
                        func_80085A14(0, v, best, 4, &v[3], 10)[9] = 0;
                        best->h12 = 0;
                    } else {
                        v[2].vx = (rand() & 0x1F) - (s16)(v[2].vx + 16);
                        v[2].vy += (rand() & 0x1F) - 16;
                        func_80014774(&v[2], &v[4]);
                        v[4].vx = v[0].vx + ((v[4].vx * 625) >> 9);
                        v[4].vy = v[0].vy + ((v[4].vy * 625) >> 9);
                        v[4].vz = v[0].vz + ((v[4].vz * 625) >> 9);
                        func_80085A14(0, v, &v[4], 4, &v[3], 10)[9] = 0;
                    }
                }
                o->w110--;
            }
        }
        a2 = D_80040FA0.h266;
        switch (o->kind) {
        case 0:
        case 1:
            a1 = D_80040FA0.h268;
            break;
        default:
            if (D_80040FA0.b27A == 0 || D_80040FA0.b27A == 3) {
                if (!(o->kind == 16 || o->kind == 176)) {
                    a1 = D_80040FA0.h268;
                } else {
                    a1 = D_80040FA0.h26C;
                }
            } else if (o->kind != 16 && o->kind == 176 && (o->vel[0] != 0 || o->vel[2] != 0)) {
                a1 = D_80040FA0.h26C;
            } else {
                a1 = D_80040FA0.h268;
            }
            break;
        }
        if (D_80040FA0.h20 & 1) {
            a1 >>= 1;
            goto recharge;
        }
        en = D_80040FA0.en;
        if (en < D_80040FA0.h26A) {
            a1 >>= 2;
            a2 >>= 2;
        }
        if ((D_80040FA0.held & D_80040FA0.k29A) && o->dir != 129 && (o->kind == 192 || o->kind == 176)) {
            if (o->kind == 176) {
                a2 >>= 1;
            }
            if (a2 < en) {
                D_80040FA0.en = en - a2;
            } else {
                D_80040FA0.en = 0;
                D_80040FA0.h20 |= 1;
            }
} else {
        recharge:
            if (D_80040FA0.en < D_80040FA0.en_max) {
                D_80040FA0.en += a1;
            } else {
                D_80040FA0.en = D_80040FA0.en_max;
                D_80040FA0.h20 &= ~1;
            }
}
        if (D_80040FA0.nweap != 0 && (D_80040FA0.held & D_80040FA0.k298) && !(D_80040FA0.prev & D_80040FA0.k298)) {
            m = D_80040FA0.nweap;
        again:
            D_80040FA0.sel++;
            if (D_80040FA0.sel >= D_80040FA0.nweap) {
                D_80040FA0.sel = 0;
            }
            m--;
            if (*(u16 *)(D_80040FA0.weap[D_80040FA0.sel] + 0x10) == 0 && m != 0)
                goto again;
            func_8004CFF4(45, 100);
            func_8007BA2C();
        }
        if (o->cool == 0 && D_80040FA0.cur != D_80040FA0.sel) {
            D_80040FA0.cur = D_80040FA0.sel;
            o->w15C &= D_80040FA0.h2BE;
            if (o->dir == 34 && D_80040FA0.b27A == 0) {
                o->dir = 35;
                o->h36 |= 0x10;
            }
            func_8007DF08();
            func_8007BA2C();
        }
        if (o->b65 != 255) {
            func_8007C790(o);
        }
        o->recoil = -1;
        if ((u32)(o->dir - 32) < 2) {
            return;
        }
        if (D_80040FA0.nweap != 0) {
            if (D_8019F524 & 0x400) {
                goto nofire;
            }
            w = D_80040FA0.weap[D_80040FA0.cur];
            switch (w[0]) {
            case 0:
                func_8007C7C8(o, w);
                break;
            case 1:
                func_8007D068(o, w);
                break;
            case 2:
                func_8007D2F8(o, w);
                break;
            }
        } else if (D_8019F524 & 0x400) {
        nofire:
            o->h148 = 0;
            o->w15C = 0;
            if (o->dir == 34 && D_80040FA0.b27A == 0) {
                o->dir = 35;
                o->h36 |= 0x10;
            }
        }
        if ((D_80040FA0.held & D_80040FA0.k29E) && !(D_80040FA0.prev & D_80040FA0.k29E)
            && !func_8008A5CC(&o->px, o->x10) && D_80040FA0.h2BA != 0 && !(D_80040FA0.h20 & 1)
            && !(o->h36 & 0x10) && D_80040FA0.en > D_80040FA0.dash_cost) {
            CHit3 *h;

            D_80040FA0.en -= D_80040FA0.dash_cost;
            v[1].vx = o->px;
            v[1].vz = o->pz;
            v[1].vy = o->py - (o->h6C >> 1);
            n = func_80073F38(&v[1], o->h12, D_80040FA0.yaw, 0, o->h36, 3000, -512, 512, -512, 512, buf);
            func_8007BA2C();
            D_80040FA0.lock = 0;
            if (n != 0) {
                h = func_80015440(buf, n, 48, func_800743A0, 0);
                if ((h->v10.pad & 2) && (u8 *)h->t >= D_801A26B8[0] && (u8 *)h->t <= D_801A26B8[15]) {
                    D_80040FA0.lock = h->t;
                    D_80040FA0.lockv = h->v10;
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
