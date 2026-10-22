#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x10];
    u8 dir;             /* +0x10 */
    u8 pad11;
    s16 on;             /* +0x12 */
    u8 pad14[4];
    u16 range;          /* +0x18 */
    s16 t;              /* +0x1A */
} Func801C5CE0Mover;
typedef struct {
    u8 pad0[2];
    s16 lo;             /* +0x02 */
    s16 hi;             /* +0x04 */
} Func801C5CE0Def;
typedef struct {
    u8 pad0[0x12];
    s16 y;              /* +0x12 */
    u8 pad14[0x18];
    Func801C5CE0Mover *mover; /* +0x2C */
    u8 pad30[8];
    Func801C5CE0Def *def; /* +0x38 */
} Func801C5CE0Obj;
extern void func_8004CE44(s32, s32, s32, s32);

void func_801C5CE0(Func801C5CE0Obj *o, s32 snd, s32 arg2)
{
    Func801C5CE0Mover *m;
    Func801C5CE0Def *d;

    s32 v;

    m = o->mover;
    d = o->def;
    if (m->on != 0) {
        goto check;
    }
flip:
    m->t = 0;
    m->dir ^= 1;
    if (snd != 0) {
        func_8004CE44(arg2, snd, 120, 0);
    }
    return;
check:
    if (m->dir) {
        v = o->y - d->lo;
    } else {
        v = d->hi - o->y;
    }
    if (m->range < v >> 1) {
        goto flip;
    }
}
