/* ovl203 func_8007B318: versus AC fires its current weapon record r.  Aim (func_8007A5EC)
 * or clear the aim, then, when the fire buttons (player +0x29C) are held, the reload
 * timer (+0x3F) is 0 and ammo is left, fire a burst of 1/2/4 shots (r +9 & 3), each from
 * the left/right muzzle pair, spawning by weapon class (kind 0..19 / 20.. / 40.. / 60.. /
 * 70..); then start the reload and the muzzle flash. */
#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct {
    u8 unk0;
    u8 kind;            /* +0x01 */
    u8 mz_a0, mz_b0;    /* +0x02 */
    u8 mz_a1, mz_b1;    /* +0x04 */
    u8 pad6[3];
    u8 flags9;          /* +0x09 */
    signed char unkA;   /* +0x0A */
    u8 unkB;            /* +0x0B */
    u8 flagsC;          /* +0x0C */
    u8 padD[3];
    u16 count;          /* +0x10 */
    u8 pad12[4];
    s16 unk16;          /* +0x16 */
    u8 pad18[0x23 - 0x18];
    u8 reload;          /* +0x23 */
    u8 unk24[4];        /* +0x24 */
} V7BRec;

typedef struct {
    u8 unk0[4];         /* +0x00 */
    u8 unk4[4];         /* +0x04 */
    s16 unk8;           /* +0x08 */
    s16 unkA;           /* +0x0A */
    s16 unkC;           /* +0x0C */
    s16 unkE;           /* +0x0E */
    s16 unk10;          /* +0x10 */
    u8 pad12[3];
    u8 unk15;           /* +0x15 */
    u8 pad16[2];
    u8 unk18[4];        /* +0x18 */
} V7BDef;               /* 0x1C */

typedef struct { s16 v; u8 pad[0x28 - 2]; } V7BTabB;
typedef struct { s16 v; u8 pad[0x24 - 2]; } V7BTabC;
typedef struct { s16 v; u8 pad[0x18 - 2]; } V7BTabD;

typedef struct {
    u8 pad0[0x12];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x3F - 0x14];
    u8 reload;          /* +0x3F */
    u8 pad40[0x65 - 0x40];
    u8 weapon_slot;     /* +0x65 */
    u8 pad66[0x74 - 0x66];
    s16 fire_timer;     /* +0x74 */
    u16 unk76;          /* +0x76 */
    u16 unk78;          /* +0x78 */
    u16 unk7A;          /* +0x7A */
    s16 unk7C;          /* +0x7C */
    u8 pad7e[0x148 - 0x7E];
    u16 fire_flags;     /* +0x148 */
    u8 pad14a[0x14E - 0x14A];
    u8 flash_r;         /* +0x14E */
    u8 flash_g;         /* +0x14F */
    u8 flash_b;         /* +0x150 */
    u8 pad151[0x15C - 0x151];
    u32 flags;          /* +0x15C */
} V7BObj;

typedef struct {
    u8 pad0[0x08];
    u32 pad_cur;        /* +0x008 */
    u8 pad0c[0x84 - 0x0C];
    void *tgt;          /* +0x084 */
    u8 pad88[0x9A - 0x88];
    u16 unk9A;          /* +0x09A */
    u16 unk9C;          /* +0x09C */
    u16 unk9E;          /* +0x09E */
    u8 padA0[0x220 - 0xA0];
    u16 unk220;         /* +0x220 */
    u8 pad222[0x286 - 0x222];
    u16 unk286;         /* +0x286 */
    u8 pad288[0x29C - 0x288];
    u16 fire_mask;      /* +0x29C */
} V7BP;

#define P ((V7BP *)D_80039D18)

extern V7BDef D_8008A220[];
extern V7BTabB D_8008B938[];
extern V7BTabC D_8008BBD8[];
extern V7BTabD D_8008B848[];

extern void func_8007A5EC(V7BObj *o, V7BRec *r, s32 v, s32 f, s32 g);
extern s32 func_8007B27C(V7BRec *r);
extern s32 func_800732FC();
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern void func_80014AE4(s32 dx, s32 dy, s32 dz, SVECTOR *ang);
extern void func_80084B44();
extern void func_80086A98();
extern void func_80087660();
extern void func_80087E08();
extern void func_800870B8();
extern void func_80088C28(void *tgt, s32 a, V7BObj *o);

void func_8007B318(V7BObj *o, V7BRec *r)
{
    s32 n;
    s32 kind;
    s32 t;
    s32 cat;
    s32 v;
    V7BDef *def;
    s32 fx;
    s32 a, b;
    s32 k;
    V7BP *p;
    SVECTOR pr[2];
    SVECTOR spare;
    SVECTOR ang;
    SVECTOR ang2;

    n = 1;
    kind = r->kind;
    t = r->flags9 & 3;
    switch (t) {
    case 1:
        n = 2;
        break;
    case 2:
        n = 4;
        break;
    }
    cat = kind >= 20;
    if (kind >= 40)
        cat = 2;
    if (kind >= 60)
        cat = 3;
    switch (cat) {
    case 0:
        def = &D_8008A220[kind];
        v = def->unk8;
        break;
    case 1:
        v = D_8008B938[kind - 20].v;
        break;
    case 2:
        v = D_8008BBD8[kind - 40].v;
        break;
    case 3:
        v = D_8008B848[kind - 60].v;
        break;
    }
    if (!(r->flagsC & 4)) {
        func_8007A5EC(o, r, v, r->flags9 & 8, P->unk286);
    } else {
        o->unk76 = P->unk220;
        o->unk7A = 0;
        o->unk78 = 0;
        P->tgt = 0;
    }
    o->unk7C = r->unkA;
    o->fire_flags &= 0xFF0F;
    o->flags &= ~((1 << r->mz_b0) | (1 << r->mz_b1));
    if (o->weapon_slot != 255)
        return;
    if (o->reload == 0) {
        if (!(P->pad_cur & P->fire_mask) || r->count == 0)
            return;
        fx = 0;
        for (n--; n != -1; n--) {
        if (func_8007B27C(r) == 0)
            return;
        if (!(n & 1)) {
            a = r->mz_a0;
            b = r->mz_b0;
        } else {
            a = r->mz_a1;
            b = r->mz_b1;
        }
        o->flags |= 1 << b;
        p = P;
        k = n & 2;
        if ((r->flags9 & 4) || (p->tgt != 0 && (p->unk9A & 8))) {
            if (func_800732FC(o, pr, a, b, 1, k) == 0)
                goto next;
            ang.vx = -P->unk220 - p->unk9C;
            ang.vy = o->yaw + p->unk9E;
            func_80014774(&ang, &pr[1]);
        } else {
            if (func_800732FC(o, pr, a, b, 2, k, k + 1) == 0)
                goto next;
            pr[1].vx -= pr[0].vx;
            pr[1].vy -= pr[0].vy;
            pr[1].vz -= pr[0].vz;
        }
        if (kind >= 70) {
            func_80014AE4(pr[1].vx, pr[1].vy, pr[1].vz, &ang);
            func_80086A98(o, pr, &pr[1], r->unk24, kind - 70, 52, &ang);
        } else if (kind >= 60) {
            func_800870B8(o, pr, &pr[1], r->unk24, r->unk16, kind - 60);
        } else if (kind >= 40) {
            func_80014AE4(pr[1].vx, pr[1].vy, pr[1].vz, &ang2);
            func_80087E08(o, pr, &pr[1], r->unk24, r->unk16, kind - 40, 49, &ang2);
        } else if (kind >= 20) {
            func_80087660(o, pr, &pr[1], r->unk24, r->unk16, kind - 20);
        } else {
            if (fx != 255)
                fx = def->unk15;
            func_80084B44(o, r->unk24, pr, &pr[1], def->unk8, r->unk16, def->unk10, def->unk18,
                          def->unkC, def->unkE, 0, def->unk0, def->unk4, def->unkA, fx);
            fx = 255;
        }
        func_80088C28(P->tgt, 1, o);
    next:
        if (--r->count == 0)
            break;
        }
        o->weapon_slot = r->unkB;
        o->fire_timer = 0;
        o->reload = r->reload;
        o->fire_flags |= 0xF0;
        o->flash_r = 180;
        o->flash_g = 160;
        o->flash_b = 140;
    } else {
        o->reload--;
    }
}
