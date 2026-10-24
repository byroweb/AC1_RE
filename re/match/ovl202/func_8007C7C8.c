/* ovl202 0x8007C7C8  register-tie tier (tools/loop/regtie.py): swap e->flash_b = 140; / e->ai_sub2 = am->cooldow + swap e->flash_g = 160; / e->ai_sub2 = am->cooldow + swap e->flash_r = 180; / e->ai_sub2 = am->cooldow + swap e->fire_timer = 0; / e->weapon_slot = a */
#include "ac1/ac1.h"

typedef struct {
    u8 b0;
    u8 id;              /* +0x01 weapon id */
    u8 mz_a0, mz_b0;    /* +0x02 muzzle pair 0 */
    u8 mz_a1, mz_b1;    /* +0x04 muzzle pair 1 */
    u8 pad6[3];
    u8 mode;            /* +0x09 &3 shots, &4 aimed, &8 */
    s8 recoil;          /* +0x0A */
    u8 slot;            /* +0x0B */
    u8 flags;           /* +0x0C */
    u8 padd[3];
    u16 count;          /* +0x10 */
    u8 pad12[4];
    s16 h16;            /* +0x16 */
    u8 pad18[0x0B];
    u8 cooldown;        /* +0x23 */
    u8 x24[4];          /* +0x24 */
} Ammo;

typedef struct {
    u8 pad0[4];
    u8 x04[4];          /* +0x04 */
    s16 range;          /* +0x08 */
    s16 hA;
    s16 hC;
    s16 hE;
    s16 h10;
    u8 pad12[3];
    u8 b15;             /* +0x15 */
    u8 pad16[2];
    u8 tracer[4];       /* +0x18 */
} WDef;

typedef struct { s16 range; u8 pad[38]; } WDef40;
typedef struct { s16 range; u8 pad[34]; } WDef36;
typedef struct { s16 range; u8 pad[22]; } WDef24;

typedef struct {
    u8 pad[0x63];
} Pad;

typedef struct Obj Obj;
struct Obj {
    u8 pad0[0x12];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x3F - 0x14];
    u8 ai_sub2;         /* +0x3F */
    u8 pad40[0x65 - 0x40];
    u8 weapon_slot;     /* +0x65 */
    u8 pad66[0x74 - 0x66];
    s16 fire_timer;     /* +0x74 */
    s16 h76;
    s16 h78;
    s16 h7A;
    s16 h7C;
    u8 pad7e[0x148 - 0x7E];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[4];
    u8 flash_r, flash_g, flash_b;   /* +0x14E */
    u8 pad151[0x0B];
    u32 flags;          /* +0x15C */
};

extern WDef D_8008E530[];
extern WDef40 D_8008FC48[];
extern WDef36 D_8008FEE8[];
extern WDef24 D_8008FB58[];
extern u16 D_8004103A;
extern u16 D_8004103C;
extern u16 D_8004103E;
extern u16 D_800411C0;
extern u16 D_80041226;
extern s32 D_80040FA8;
extern u16 D_8004123C;
extern void func_8007BA98(Obj *e, Ammo *am, s32 range, s32 aimed, s32 x);
extern s32 func_8007C730(Ammo *am);
extern s32 func_800745D0();
extern void func_80014774(SVECTOR *rot, SVECTOR *dir);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);
extern void func_80087DEC(Obj *e, SVECTOR *m, SVECTOR *d, void *x, s32 a, s32 b, SVECTOR *c);
extern void func_8008840C(Obj *e, SVECTOR *m, SVECTOR *d, void *x, s32 a, s32 b);
extern void func_8008915C(Obj *e, SVECTOR *m, SVECTOR *d, void *x, s32 a, s32 b, s32 c, SVECTOR *f);
extern void func_800889B4(Obj *e, SVECTOR *m, SVECTOR *d, void *x, s32 a, s32 b);
extern void func_80085E98(Obj *e, void *x, SVECTOR *m, SVECTOR *d, s32 range, s32 a, s32 b, void *tr, s32 c, s32 dd, s32 z, WDef *w, void *w4, s32 f, s32 g);
extern void func_80089F7C(void *t, s32 a, Obj *e);

void func_8007C7C8(Obj *e, Ammo *am)
{
    SVECTOR m;
    SVECTOR d;
    SVECTOR unused;
    SVECTOR a7;
    SVECTOR a6;
    WDef *w;
    s32 id;
    s32 n;
    s32 cat;
    s32 range;
    s32 prev;
    s32 k;
    s32 ma;
    s32 mb;
    void **tp;

    n = 1;
    id = am->id;
    switch (am->mode & 3) {
    case 1:
        n = 2;
        break;
    case 2:
        n = 4;
        break;
    }
    cat = id >= 20;
    if (id >= 40) {
        cat = 2;
    }
    if (id >= 60) {
        cat = 3;
    }
    switch (cat) {
    case 0:
        w = &D_8008E530[id];
        range = w->range;
        break;
    case 1:
        range = D_8008FC48[id - 20].range;
        break;
    case 2:
        range = D_8008FEE8[id - 40].range;
        break;
    case 3:
        range = D_8008FB58[id - 60].range;
        break;
    }
    if (!(am->flags & 4)) {
        func_8007BA98(e, am, range, am->mode & 8, D_80041226);
    } else {
        e->h7A = 0;
        e->h78 = 0;
        e->h76 = D_800411C0;
        g_target_list[0].target = 0;
    }
    e->h7C = am->recoil;
    e->fire_flags &= 0xFF0F;
    e->flags &= ~((1 << am->mz_b0) | (1 << am->mz_b1));
    if (e->weapon_slot != 255) {
        return;
    }
    if (e->ai_sub2 == 0) {
        if (!(D_80040FA8 & D_8004123C)) {
            return;
        }
        if (am->count == 0) {
            return;
        }
        prev = 0;
        while (--n != -1) {
            if (func_8007C730(am) == 0) {
                return;
            }
            if (!(n & 1)) {
                ma = am->mz_a0;
                mb = am->mz_b0;
            } else {
                ma = am->mz_a1;
                mb = am->mz_b1;
            }
            e->flags |= 1 << mb;
            tp = (void **)&g_target_list[0].target;
            k = n & 2;
            if ((am->mode & 4) || (*tp != 0 && (D_8004103A & 8))) {
                if (func_800745D0(e, &m, ma, mb, 1, k) == 0) {
                    goto next;
                }
                a7.vx = -D_800411C0 - D_8004103C;
                a7.vy = e->yaw + D_8004103E;
                func_80014774(&a7, &d);
            } else {
                if (func_800745D0(e, &m, ma, mb, 2, k, k + 1) == 0) {
                    goto next;
                }
                d.vx -= m.vx;
                d.vy -= m.vy;
                d.vz -= m.vz;
            }
            if (id >= 70) {
                func_80014AE4(d.vx, d.vy, d.vz, &a7);
                func_80087DEC(e, &m, &d, am->x24, id - 70, 52, &a7);
            } else if (id >= 60) {
                func_8008840C(e, &m, &d, am->x24, am->h16, id - 60);
            } else if (id >= 40) {
                func_80014AE4(d.vx, d.vy, d.vz, &a6);
                func_8008915C(e, &m, &d, am->x24, am->h16, id - 40, 49, &a6);
            } else if (id >= 20) {
                func_800889B4(e, &m, &d, am->x24, am->h16, id - 20);
            } else {
                if (prev != 255) {
                    prev = w->b15;
                }
                func_80085E98(e, am->x24, &m, &d, w->range, am->h16, w->h10, w->tracer, w->hC, w->hE, 0, w, w->x04, w->hA, prev);
                prev = 255;
            }
            func_80089F7C(g_target_list[0].target, 1, e);
        next:
            if (--am->count == 0) {
                break;
            }
        }
        e->weapon_slot = am->slot;
        e->fire_timer = 0;
        e->ai_sub2 = am->cooldown;
        e->fire_flags |= 0xF0;
        e->flash_r = 180;
        e->flash_g = 160;
        e->flash_b = 140;
    } else {
        e->ai_sub2--;
    }
}
