#include "ac1/ac1.h"

typedef struct {
    void *target;       /* +0x00 */
    s32 lock;           /* +0x04 */
    u16 h8;             /* +0x08 lock-on progress */
    s16 hA;             /* +0x0A */
    s16 hC;
    s16 hE;
    SVECTOR proj;       /* +0x10 (flags at +0x16 = proj.pad) */
    s16 aim[4];         /* +0x18 (+0x1E counter) */
    SVECTOR prev;       /* +0x20 (+0x26 h26) */
    SVECTOR d;          /* +0x28 */
} TSlot;

typedef struct {
    u8 b0, id;
    u8 mz_a0, mz_b0;    /* +0x02 */
    u8 pad4[0x10];
    s16 h14;            /* +0x14 */
    u8 pad16[2];
    s16 h18;            /* +0x18 */
    s16 h1A;            /* +0x1A */
    s16 h1C;            /* +0x1C */
    s16 h1E;            /* +0x1E */
    s16 h20;            /* +0x20 */
} Ammo;

typedef struct {
    u8 pad0[0x10];
    u8 h10[2];          /* +0x10 */
    s16 yaw;            /* +0x12 */
    u8 pad14[0x36 - 0x14];
    u16 faction;        /* +0x36 */
    u8 pad38[0x76 - 0x38];
    s16 h76;
    s16 h78;
    s16 h7A;
} Obj;

typedef struct {
    TSlot *head;        /* 0x80041020 */
    TSlot list[8];      /* 0x80041024 */
} TgtState;

#define slot0 (D_80041020.list[0])

extern TgtState D_80041020;
extern s16 D_800411C0;
extern s32 D_801A5DC4;
extern SVECTOR D_80039C78;
extern s32 func_800745D0();
extern s32 func_80073F38(SVECTOR *from, s16 yaw, s16 pitch, u16 mask, s32 team, s32 range,
                         s16 xmin, s16 xmax, s16 ymin, s16 ymax, TSlot *slot);
extern TSlot *func_80015440(TSlot *base, s32 n, s32 size, void *cmp, void *x);
extern s32 func_800743A0();
extern long SquareRoot0(long a);
extern void func_80015258(SVECTOR *a, void *b, s32 c, SVECTOR *d, SVECTOR *e, SVECTOR *f, s16 *g, s32 h);
extern void func_8004CFF4(s32 a, s32 b);

void func_8007BA98(Obj *e, Ammo *am, s32 range, s32 aimed, s32 mask)
{
    TSlot cand[16];
    SVECTOR m;
    u16 oldflags;
    TSlot *t;
    TSlot *p;
    TgtState *g;
    s32 n;
    s32 s;
    s32 k;
    u16 fl;

    g = &D_80041020;
    p = g->list;
    g->head = p;
    oldflags = p->proj.pad;
    if (func_800745D0(e, &m, am->mz_a0, am->mz_b0, 1, 0) == 0) {
        goto clear;
    }
    n = func_80073F38(&m, e->yaw, D_800411C0, mask, e->faction, am->h14, -am->h18, am->h18, -am->h1A, am->h1A, cand);
    if (n == 0) {
        goto clear;
    }
    t = func_80015440(cand, n, 48, func_800743A0, p->target);
    if (p->target == t->target) {
        t->d.vx = t->proj.vx - p->proj.vx;
        t->d.vy = t->proj.vy - p->proj.vy;
        t->d.vz = t->proj.vz - p->proj.vz;
        p->proj.vx = t->proj.vx;
        p->proj.vy = t->proj.vy;
        p->proj.vz = t->proj.vz;
        p->aim[0] = t->aim[0];
        p->aim[1] = t->aim[1];
        p->aim[2] = t->aim[2];
        p->prev = p->proj;
        fl = t->proj.pad;
        p->proj.pad = fl;
        p->lock = t->lock;
        if (p->aim[3] < 16384) {
            p->aim[3]++;
        }
        if (!(fl & 2)) {
            goto unlock;
        }
        if (t->aim[0] < -am->h1C || t->aim[0] > am->h1C) {
            goto unlock;
        }
        if (t->aim[1] < -am->h1E || t->aim[1] > am->h1E) {
            goto unlock;
        }
        p->proj.pad = fl | 4;
        s = SquareRoot0(t->d.vx * t->d.vx + t->d.vy * t->d.vy + t->d.vz * t->d.vz);
        s = (s * am->h20) >> 8;
        s = (s * p->lock) >> 15;
        p->prev.pad = s - p->h8;
        if ((s16)(s - p->h8) <= 0) {
            p->h8 = s + 32;
            k = p->hA >> 2;
            if (k >= 9) {
                k = 8;
            }
            func_80015258(&m, e->h10, range, &p->proj, &t->d, &p->prev, p->aim, k);
            p->aim[0] -= D_800411C0;
            p->prev.pad = s - p->hA;
            if (p->hA < 16384) {
                p->hA++;
            }
            if (p->hA >= s) {
                p->proj.pad |= 8;
                if (oldflags & 8) {
                    goto out;
                }
                func_8004CFF4(42, 100);
                goto out;
            }
        } else {
            p->prev.pad = (s - p->h8) + s;
            if ((s16)p->h8 < 16384) {
                p->h8++;
            }
            p->hA = 0;
        }
        goto beep;
    unlock:
        p->hA = 0;
        p->h8 = 0;
    beep:
        if (D_801A5DC4 & 1) {
            goto out;
        }
        func_8004CFF4(46, 100);
    } else {
        *p = *t;
        p->d = D_80039C78;
        p->prev = p->proj;
        p->hE = 0;
        p->hC = 0;
        p->hA = 0;
        p->h8 = 0;
        p->aim[3] = 0;
        p->proj.pad &= ~8;
    }
out:
    if (p->proj.pad & 2) {
        if (aimed) {
            e->h76 = p->aim[0] + D_800411C0;
            e->h78 = -p->aim[1];
            e->h7A = 0;
        } else {
            e->h76 = p->aim[0] + D_800411C0;
            e->h7A = -p->aim[1];
            e->h78 = 0;
        }
        return;
    }
clear:
    e->h7A = 0;
    e->h78 = 0;
    e->h76 = D_800411C0;
    p->target = 0;
}
