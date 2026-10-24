/* ovl203 0x8007AB4C  register-tie tier (tools/loop/regtie.py): reuse k2 for k from k = n;
    do { + split k2 at k2 = (r->rate * 3) >> 4; */
/* ovl203 func_8007AB4C: versus multi-lock aim (missile weapon record r).  Scan for targets
 * from the muzzle point; drop locked slots that left the window, refresh the ones still
 * seen, add the first new lockable target to every free slot; count the locks (random
 * lock loss on jamming ACs), pick the slot to work on (func_80015440 / func_800731C8),
 * turn the lock cursor onto it and advance its lock timer (tones 43/44, 47). */
#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct {
    s16 x, y, z;
    s16 w;
} V7BVec;

typedef struct {
    void *target;       /* +0x00 */
    s32 lock;           /* +0x04 */
    s16 t08;            /* +0x08 */
    s16 t0a;            /* +0x0A */
    s16 t0c;            /* +0x0C */
    s16 t0e;            /* +0x0E */
    V7BVec pos;         /* +0x10 (w = flags) */
    s16 aim[4];         /* +0x18 */
    s16 prev[3];        /* +0x20 */
    s16 nlock;          /* +0x26 */
    SVECTOR vel;        /* +0x28 */
} V7BSlot;

typedef struct {
    u8 pad0[2];
    u8 mz_a;            /* +0x02 */
    u8 mz_b;            /* +0x03 */
    u8 pad4[5];
    u8 lock_time;       /* +0x09 */
    u8 padA[6];
    u16 maxlock2;       /* +0x10 */
    u8 pad12[2];
    s16 range;          /* +0x14 */
    u8 pad16[2];
    s16 ywin2;          /* +0x18 */
    s16 pwin2;          /* +0x1A */
    s16 pwin;           /* +0x1C */
    s16 ywin;           /* +0x1E */
    s16 rate;           /* +0x20 */
    u8 maxlock;         /* +0x22 */
} V7BRec;

typedef struct {
    u8 pad0[0x12];
    s16 yaw;            /* +0x12 */
    u8 pad14[0x36 - 0x14];
    u16 faction;        /* +0x36 */
} V7BObj;

typedef struct {
    u8 pad0[0x38];
    u16 flags38;        /* +0x38 */
    u8 pad3a[0x170 - 0x3A];
} V7BEnt;

typedef struct {
    u8 pad0[0x22];
    u16 nlock;          /* +0x022 */
    s32 unk24;          /* +0x024 */
    s16 ax;             /* +0x028 */
    s16 ay;             /* +0x02A */
    s16 unk2C;          /* +0x02C */
    s16 on;             /* +0x02E */
    s16 pt[3];          /* +0x030 */
    u8 pad36[0x80 - 0x36];
    V7BSlot *head;      /* +0x080 */
    V7BSlot slot[8];    /* +0x084 */
    u8 pad204[0x220 - 0x204];
    s16 unk220;         /* +0x220 */
    u8 pad222[0x286 - 0x222];
    u16 unk286;         /* +0x286 */
} V7BP;

#define P ((V7BP *)D_80039D18)

typedef struct {
    void *target;
    s32 unk24;
} V7BKey;

extern V7BEnt D_8019B550[];
extern struct { s32 v; } D_8019D83C;
extern s32 func_800732FC();
extern s32 func_80072C98(SVECTOR *p, s16 yaw, s16 pitch, u16 mask, s32 team, s32 range,
                         s16 pmin, s16 pmax, s16 ymin, s16 ymax, V7BSlot *slot);
extern V7BSlot *func_80015440(V7BSlot *list, s32 n, s32 size, s32 (*cmp)(), V7BKey *key);
extern s32 func_800731C8();
extern void func_8007A590(void);
extern s16 func_800132CC(s16 a, s16 b, s16 c);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern s32 SquareRoot0(s32 v);
extern s32 rand(void);
extern void func_8004D1FC(s32 id, s32 vol);

void func_8007AB4C(V7BObj *o, V7BRec *r)
{
    V7BSlot list[2];
    SVECTOR mp;
    SVECTOR spare;
    V7BKey key;
    SVECTOR ang;
    SVECTOR dir;
    V7BSlot *sl;
    V7BSlot *sl4;
    V7BSlot *l;
    V7BEnt *t;
    s32 n;
    s32 i;
    s32 k;
    s32 k2;
    s32 k2_b;
    s32 total;
    s32 d;
    s32 snd;

    if (func_800732FC(o, &mp, r->mz_a, r->mz_b, 1, 0) != 0
        && (n = func_80072C98(&mp, o->yaw, P->unk220, P->unk286, o->faction, r->range,
                              -r->pwin, r->pwin, -r->ywin, r->ywin, list)) != 0) {
    for (i = 8, sl = P->slot; i != 0; sl++, i--) {
        if (sl->target != 0
            && (sl->aim[1] < -r->pwin2 || r->pwin2 < sl->aim[1]
                || sl->aim[0] < -r->ywin2 || r->ywin2 < sl->aim[0]))
            sl->target = 0;
    }
    for (i = 8, sl = P->slot; i != 0; sl++, i--) {
        if (sl->target == 0)
            continue;
        l = list;
        k2 = n;
        do {
            if (sl->target == l->target) {
                sl->lock = l->lock;
                sl->pos.x = l->pos.x;
                sl->pos.y = l->pos.y;
                sl->pos.z = l->pos.z;
                sl->aim[0] = l->aim[0];
                sl->aim[1] = l->aim[1];
                sl->aim[2] = l->aim[2];
                sl->pos.w = (sl->pos.w & ~3) | (l->pos.w & 3);
                l->target = 0;
                goto next;
            }
            l++;
        } while (--k2 != 0);
        sl->target = 0;
    next:;
    }
    l = list;
    k2 = n;
    do {
        if (l->target != 0 && (l->pos.w & 2)) {
            for (i = 8, sl = P->slot; i != 0; sl++, i--) {
                if (sl->target == 0) {
                    sl->target = l->target;
                    sl->lock = l->lock;
                    sl->pos = l->pos;
                    *(SVECTOR *)sl->aim = *(SVECTOR *)l->aim;
                    sl->t0e = 0;
                    sl->t0c = 0;
                    sl->nlock = 0;
                    sl->t0a = 0;
                    sl->t08 = 0;
                }
            }
            break;
        }
        l++;
    } while (--k2 != 0);
    total = 0;
    for (i = 8, sl4 = P->slot; i != 0; sl4++, i--) {
        if (sl4->target == 0)
            continue;
        if (rand() < 3277) {
            t = sl4->target;
            if ((u32)t >= (u32)D_8019B550 && (u32)&D_8019B550[1] >= (u32)t && (t->flags38 & 8)) {
                sl4->nlock = 0;
                sl4->pos.w &= 0xFFE7;
            }
        }
        if (sl4->pos.w & 8)
            total += sl4->nlock;
    }
    if (total >= r->maxlock || total >= r->maxlock2) {
        P->on = 0;
        P->nlock = 0;
        return;
    }
    P->nlock = r->maxlock - total;
    key.target = P->head->target;
    key.unk24 = P->unk24;
    l = func_80015440(P->slot, 8, 48, func_800731C8, &key);
    if (P->head->target != l->target)
        P->unk24 = 0;
    P->head = l;
    if (l->target != 0 && (l->pos.w & 2) && l->nlock < 6) {
    k2_b = (r->rate * 3) >> 4;
    P->ax = func_800132CC(P->ax, l->aim[0], k2_b);
    P->ay = func_800132CC(P->ay, l->aim[1], k2_b);
    ang.vx = -P->unk220 - P->ax;
    ang.vy = o->yaw + P->ay;
    func_80014774(&ang, &dir);
    d = SquareRoot0((l->pos.x - mp.vx) * (l->pos.x - mp.vx)
                    + (l->pos.y - mp.vy) * (l->pos.y - mp.vy)
                    + (l->pos.z - mp.vz) * (l->pos.z - mp.vz));
    P->pt[0] = mp.vx + ((d * dir.vx) >> 12);
    P->pt[1] = mp.vy + ((d * dir.vy) >> 12);
    P->pt[2] = mp.vz + ((d * dir.vz) >> 12);
    P->on = 1;
    if ((*(s32 *)&P->ax & 0x0FFF0FFF) != (*(s32 *)l->aim & 0x0FFF0FFF))
        return;
    if (r->lock_time < l->t08) {
        l->pos.w |= 0x18;
        if (l->nlock < 6) {
            l->nlock++;
            snd = 43;
            if (P->nlock == 1)
                snd = 44;
            func_8004D1FC(snd, 100);
        }
        P->on = 0;
        P->unk2C = 0;
        P->ay = 0;
        P->ax = P->unk220;
        l->t08 = 0;
        return;
    }
    l->t08++;
    if (!(D_8019D83C.v & 3))
        func_8004D1FC(47, 100);
    } else {
        P->on = 0;
    }
    } else {
        func_8007A590();
        P->nlock = r->maxlock;
    }
}
