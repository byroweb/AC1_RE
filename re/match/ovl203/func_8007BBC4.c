#include "ac1/ac1.h"
/* ovl203 weapon lock/fire step for the side in D_80039D18 (cf. ovl202 0x8007D068, same shape) */
typedef struct {
    u8 pad0[0x3E];
    u8 cnt;             /* +0x3E */
    u8 delay;           /* +0x3F */
    u8 pad40[0x76 - 0x40];
    SVECTOR vec;        /* +0x76 */
} WObj3;
typedef struct {
    u8 pad0;
    u8 idx;             /* +0x01 */
    u8 a0, a1, b0, b1;  /* +0x02..+0x05 */
    u8 pad6[4];
    s8 pad_v;           /* +0x0A */
    u8 padb;
    u8 bC;              /* +0x0C */
    u8 padd[0x23 - 0x0D];
    u8 delay;           /* +0x23 */
} WDef3;
typedef struct {
    u8 pad0[2];
    u8 a;               /* +0x02 */
    u8 b;               /* +0x03 */
    u8 pad4[4];
} WEnt3;
typedef struct {        /* 0x30 */
    void *target;       /* +0x00 */
    u8 pad4[0x16 - 4];
    u16 flags;          /* +0x16 */
    u8 pad18[0x26 - 0x18];
    s16 n;              /* +0x26 */
    u8 pad28[8];
} TSlot3;
typedef struct {
    u8 pad0[6];
    s16 f6;
} Lock3;
typedef struct {
    u8 pad0[8];
    u32 flags;          /* +0x08 */
    u8 padc[0x22 - 0xC];
    u16 h22;            /* +0x22 */
    Lock3 *lock;        /* +0x24 */
    u8 pad28[0x80 - 0x28];
    TSlot3 *best;       /* +0x80 */
    TSlot3 slot[8];     /* +0x84 */
    u8 pad204[0x29C - 0x204];
    u16 mask;           /* +0x29C */
} Side3;
extern Side3 *D_80039D18;
extern SVECTOR D_80039C78;
extern WEnt3 D_8008A300[];
extern void func_8007AB4C(WObj3 *o, WDef3 *w, WEnt3 *e);
extern TSlot3 *func_8007B8CC(WObj3 *o, WDef3 *w, WEnt3 *e, s32 a, s32 b, s32 c);
extern void func_80088C28(void *t, s32 a, WObj3 *o);

void func_8007BBC4(WObj3 *o, WDef3 *w)
{
    WEnt3 *e;
    TSlot3 *t;
    TSlot3 *best;
    TSlot3 *hit;
    s32 i;
    s32 max;

    e = &D_8008A300[w->idx];
    o->vec = D_80039C78;
    o->vec.pad = w->pad_v;
    if (o->delay == 0) {
        if (o->cnt == 0) {
            if (D_80039D18->lock != 0 && D_80039D18->lock->f6 == -1) {
                D_80039D18->lock = 0;
            }
            func_8007AB4C(o, w, e);
            best = 0;
            if ((D_80039D18->flags & D_80039D18->mask) || (D_80039D18->h22 == 0 && (w->bC & 0x80))) {
                max = 0;
                i = 8;
                t = D_80039D18->slot;
                do {
                    if (t->target != 0 && (t->flags & 8)) {
                        if (max < t->n) {
                            best = t;
                            max = t->n;
                        }
                        o->cnt += t->n;
                    }
                    i--;
                    t++;
                } while (i != 0);
                if (o->cnt != 0) {
                    D_80039D18->lock = 0;
                    D_80039D18->best = best;
                    o->cnt *= 4;
                }
            }
        } else {
            if (!(o->cnt & 3)) {
                hit = func_8007B8CC(o, w, e, e->a, w->a0, w->a1);
                func_8007B8CC(o, w, e, e->b, w->b0, w->b1);
                if (hit != 0) {
                    func_80088C28(D_80039D18->slot[0].target, 2, o);
                    if (--hit->n == 0) {
                        hit->flags &= ~8;
                    }
                }
            }
            if (--o->cnt == 0) {
                o->delay = w->delay;
            }
        }
    } else {
        o->delay--;
    }
}
