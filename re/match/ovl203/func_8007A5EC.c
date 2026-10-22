/* ovl203 func_8007A5EC: versus lock-on / aim for weapon record r.  Scan for targets
 * (func_80072C98) from the muzzle point, pick one (func_80015440 with func_800730CC);
 * keep tracking the current target (velocity, lock timers, lead via func_80015258,
 * lock tone 42 / scan tone 46) or switch to the new one; then aim the AC (+0x76..+0x7A)
 * at it, or clear the aim when nothing is locked. */
#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct {
    s16 x, y, z;
    u16 w;
} V7AVec;

typedef struct {
    void *target;       /* +0x00 */
    s32 lock;           /* +0x04 */
    u16 t08;            /* +0x08 */
    s16 t0a;            /* +0x0A */
    s16 t0c;            /* +0x0C */
    s16 t0e;            /* +0x0E */
    V7AVec pos;         /* +0x10 (w = flags) */
    s16 aim[4];         /* +0x18 */
    V7AVec prev;        /* +0x20 */
    SVECTOR vel;        /* +0x28 */
} V7ASlot;

typedef struct {
    u8 pad0[2];
    u8 mz_a;            /* +0x02 */
    u8 mz_b;            /* +0x03 */
    u8 pad4[0x14 - 4];
    s16 range;          /* +0x14 */
    u8 pad16[2];
    s16 pwin;           /* +0x18 */
    s16 ywin;           /* +0x1A */
    s16 pcone;          /* +0x1C */
    s16 ycone;          /* +0x1E */
    s16 lockk;          /* +0x20 */
} V7ARec;

typedef struct {
    u8 pad0[0x10];
    s16 unk10;          /* +0x10 */
    s16 yaw;            /* +0x12 */
    u8 pad14[0x36 - 0x14];
    u16 faction;        /* +0x36 */
    u8 pad38[0x76 - 0x38];
    u16 unk76;          /* +0x76 */
    u16 unk78;          /* +0x78 */
    u16 unk7A;          /* +0x7A */
} V7AObj;

typedef struct {
    u8 pad0[0x80];
    V7ASlot *head;      /* +0x080 */
    V7ASlot cur;        /* +0x084 */
    u8 padb4[0x220 - 0xB4];
    s16 unk220;         /* +0x220 */
} V7AP;

#define P ((V7AP *)D_80039D18)

extern s32 D_8019D83C;
extern SVECTOR D_80039C78;
extern s32 func_800732FC();
extern s32 func_80072C98(SVECTOR *p, s16 yaw, s16 pitch, u16 mask, s32 team, s32 range,
                         s16 pmin, s16 pmax, s16 ymin, s16 ymax, V7ASlot *slot);
extern V7ASlot *func_80015440(V7ASlot *list, s32 n, s32 size, s32 (*cmp)(), void *key);
extern s32 func_800730CC();
extern s32 SquareRoot0(s32 v);
extern void func_80015258(SVECTOR *a, s16 *b, s32 c, V7AVec *d, SVECTOR *e, V7AVec *f, s16 *g, s32 h);
extern void func_8004D1FC(s32 id, s32 vol);

void func_8007A5EC(V7AObj *o, V7ARec *r, s32 v, s32 side, s32 mask)
{
    V7AP *p;
    V7ASlot *cur;
    V7ASlot *best;
    u16 saved;
    s32 n;
    s32 lk;
    s32 m;
    s32 snd;
    s16 d;
    V7ASlot list[2];
    SVECTOR mp;

    p = D_80039D18;
    cur = &p->cur;
    p->head = cur;
    saved = p->cur.pos.w;
    if (func_800732FC(o, &mp, r->mz_a, r->mz_b, 1, 0) == 0)
        goto none;
    n = func_80072C98(&mp, o->yaw, P->unk220, mask, o->faction, r->range,
                      -r->pwin, r->pwin, -r->ywin, r->ywin, list);
    if (n == 0)
        goto none;
    best = func_80015440(list, n, 48, func_800730CC, p->cur.target);
    if (p->cur.target == best->target) {
        best->vel.vx = best->pos.x - p->cur.pos.x;
        best->vel.vy = best->pos.y - p->cur.pos.y;
        best->vel.vz = best->pos.z - p->cur.pos.z;
        p->cur.pos.x = best->pos.x;
        p->cur.pos.y = best->pos.y;
        p->cur.pos.z = best->pos.z;
        p->cur.aim[0] = best->aim[0];
        p->cur.aim[1] = best->aim[1];
        p->cur.aim[2] = best->aim[2];
        p->cur.prev = p->cur.pos;
        p->cur.pos.w = best->pos.w;
        p->cur.lock = best->lock;
        if (p->cur.aim[3] < 0x4000)
            p->cur.aim[3]++;
        if ((p->cur.pos.w & 2) && best->aim[0] >= -r->pcone && r->pcone >= best->aim[0]
            && best->aim[1] >= -r->ycone && r->ycone >= best->aim[1]) {
            p->cur.pos.w |= 4;
            lk = (SquareRoot0(best->vel.vx * best->vel.vx + best->vel.vy * best->vel.vy
                              + best->vel.vz * best->vel.vz) * r->lockk) >> 8;
            lk = (lk * p->cur.lock) >> 15;
            d = lk - p->cur.t08;
            p->cur.prev.w = d;
            if (d <= 0) {
                p->cur.t08 = lk + 32;
                m = p->cur.t0a >> 2;
                if (m >= 9)
                    m = 8;
                func_80015258(&mp, &o->unk10, v, &p->cur.pos, &best->vel, &p->cur.prev, p->cur.aim, m);
                p->cur.aim[0] -= P->unk220;
                p->cur.prev.w = lk - p->cur.t0a;
                if (p->cur.t0a < 0x4000)
                    p->cur.t0a++;
                if (p->cur.t0a >= lk) {
                    p->cur.pos.w |= 8;
                    if (saved & 8)
                        goto aim;
                    snd = 42;
                    goto play;
                }
            } else {
                p->cur.prev.w = d + lk;
                if ((s16)p->cur.t08 < 0x4000)
                    p->cur.t08++;
                p->cur.t0a = 0;
            }
        } else {
            cur->t0a = 0;
            cur->t08 = 0;
        }
        if (D_8019D83C & 1)
            goto aim;
        snd = 46;
    play:
        func_8004D1FC(snd, 100);
    } else {
        *cur = *best;
        cur->vel = D_80039C78;
        cur->prev = cur->pos;
        cur->t0e = 0;
        cur->t0c = 0;
        cur->t0a = 0;
        cur->t08 = 0;
        cur->aim[3] = 0;
        cur->pos.w &= 0xFFF7;
    }
aim:
    if (cur->pos.w & 2) {
        if (side) {
            o->unk76 = cur->aim[0] + P->unk220;
            o->unk78 = -cur->aim[1];
            o->unk7A = 0;
        } else {
            o->unk76 = cur->aim[0] + P->unk220;
            o->unk7A = -cur->aim[1];
            o->unk78 = 0;
        }
        return;
    }
none:
    o->unk76 = P->unk220;
    o->unk7A = 0;
    o->unk78 = 0;
    cur->target = 0;
}
