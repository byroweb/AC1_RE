/* fdat002 (17 twins): world-object trigger state step (func_801C6324 calls it with the
 * stats +7 condition).  Conditions 0..3 turn the object about axis cond (through
 * func_8001380C), 4 and above move it along +0x0A down (4) or up.  States 1 and 4 run
 * for +0x12 steps and then advance by 2; state 2 runs the same step reversed and counts
 * back down, re-arming the linked world object (flag 0x100) at 0 (returns 2) and, once
 * below +0x13, re-triggering the chain when the player is clear.  State 3 triggers the
 * chain with state 2 when the player is clear.  Returns the new phase (0, 1 or 2).
 * The three speed picks use an s16 temp each for the plain copies (lhu) and an int for
 * the negated one (lh). */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[8];
    u8 rot[2];          /* +0x08 */
    s16 move;           /* +0x0A */
    u8 padc[4];
    u8 state;           /* +0x10 */
    u8 count;           /* +0x11 */
    u8 steps;           /* +0x12 */
    u8 rearm;           /* +0x13 */
    u8 pad14[6];
    s16 speed_on;       /* +0x1A */
    s16 speed_off;      /* +0x1C */
    s16 timer;          /* +0x1E */
    u16 tick;           /* +0x20 */
} Func801C6064Hit;
typedef struct {
    u8 pad0[0x0A];
    u8 locked;          /* +0x0A */
} Func801C6064Stats;
typedef struct {
    u8 pad0[0x16];
    s16 axis;           /* +0x16 */
    u8 pad18[0x28 - 0x18];
    u16 *ids;           /* +0x28 */
    Func801C6064Hit *on_hit; /* +0x2C */
    u32 flags;          /* +0x30 */
    u8 pad34[4];
    Func801C6064Stats *stats; /* +0x38 */
    u8 pad3c[4];
} Func801C6064World;
typedef struct {
    u8 pad0[4];
    s16 speed;          /* +0x04 */
} Func801C6064Turn;
extern Func801C6064World D_801D0B68[];
extern Func801C6064Turn D_801C6DD4;
extern s16 D_801C6DD8; /* D_801C6DD4.speed, written as a scalar */
extern void func_8001380C(Func801C6064Turn *t, u8 *rot, s32 axis, s32 sign);
extern void func_80072180(Func801C6064World *w);
extern s32 func_801C6014(Func801C6064Hit *h);
extern void func_801C5EA8(Func801C6064World *w, s32 state);
s32 func_801C6064(Func801C6064World *w, s32 cond, s32 axis, s32 sign)
{
    Func801C6064Hit *h = w->on_hit;
    s32 turn, t;
    s16 u, v;

    turn = 1;
    if (cond >= 4) {
        turn = 0;
        sign = 1;
        if (cond == 4)
            sign = -1;
    } else {
        axis = cond;
    }
    switch (h->state) {
    case 2:
        axis += 2;
        sign = -sign;
        /* fall through */
    case 1:
    case 4:
        if (turn) {
            axis = (axis + w->axis) & 3;
            h->timer -= h->tick;
            if (h->timer <= 0) {
                h->timer += h->steps;
                u = h->speed_off;
            } else {
                u = h->speed_on;
            }
            D_801C6DD8 = u;
            func_8001380C(&D_801C6DD4, h->rot, axis, sign);
        } else if (sign > 0) {
            h->timer -= h->tick;
            if (h->timer <= 0) {
                h->timer += h->steps;
                v = h->speed_off;
            } else {
                v = h->speed_on;
            }
            h->move = v;
        } else {
            h->timer -= h->tick;
            if (h->timer <= 0) {
                h->timer += h->steps;
                t = -h->speed_off;
            } else {
                t = -h->speed_on;
            }
            h->move = t;
        }
        func_80072180(w);
        if (h->state == 1 || h->state == 4) {
            if (++h->count >= h->steps)
                h->state += 2;
            return 1;
        }
        if (--h->count == 0) {
            h->state = 0;
            D_801D0B68[*w->ids].flags |= 0x100;
            return 2;
        }
        if (h->count < h->rearm)
            return 1;
        if (w->stats->locked != 0)
            return 1;
        if (func_801C6014(h))
            func_801C5EA8(w, 1);
        return 1;
    case 3:
        if (w->stats->locked != 0)
            return 0;
        if (func_801C6014(h))
            return 0;
        func_801C5EA8(w, 2);
        return 0;
    }
    return 0;
}
