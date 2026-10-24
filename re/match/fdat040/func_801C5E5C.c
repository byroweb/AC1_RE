#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0xA];
    s16 spd;            /* +0x0A */
    u8 pad0c[4];
    u8 dir;             /* +0x10 */
    u8 pad11;
    s16 w12;            /* +0x12 */
    s16 acc;            /* +0x14 */
    u16 vmax;           /* +0x16 */
    u16 range;          /* +0x18 */
    u16 cnt;            /* +0x1A */
} Mover;
typedef struct {
    u8 pad0[2];
    s16 lo;             /* +0x02 */
    s16 hi;             /* +0x04 */
    u8 pad6[2];
    u8 snd8;            /* +0x08 */
    u8 snd9;            /* +0x09 */
} Def;
typedef struct {
    u8 pad0[0x10];
    u8 pad10[2];
    s16 y;              /* +0x12 */
    u8 pad14[0x18];
    Mover *m;           /* +0x2C */
    u8 pad30[8];
    Def *def;           /* +0x38 */
} Obj;
extern void func_8004CC30();
extern void func_8004CE44();
extern s32 func_80072A00();
extern void func_801C5CE0();
extern u8 D_801D0B68[];

void func_801C5E5C(Obj *o)
{
    Mover *m;
    Def *d;
    u8 dir;
    s32 v;
    s32 a;
    s32 b;
    s32 w;
    s32 x;
    s8 idx;
    s32 s;

    m = o->m;
    dir = m->dir;
    d = o->def;
    if (dir != 0) {
        x = m->w12;
        v = d->hi - o->y;
    } else {
        v = o->y - d->lo;
        x = -m->w12;
    }
    if (v < m->range) {
        b = x - m->acc;
        if (b <= 0) {
            if (v == 0) {
                if (m->spd != 0) {
                    m->spd = 0;
                    func_8004CE44(o->def->snd9, (u8 *)o + 0x10, 0x78, 0);
                }
            } else {
                x = v;
                if (v >= 4) {
                    x = 4;
                }
                w = -x;
                if (dir != 0) {
                    m->spd = x;
                } else {
                    goto blk17;
                }
                goto blk18;
            }
        } else {
            goto blk14;
        }
    } else {
        u16 t = m->vmax;
        b = x + m->acc;
        if ((s32)t < b) {
            b = t;
        }
    blk14:
        if (dir != 0) {
            w = (s16)b >> 5;
            m->w12 = b;
        } else {
            m->w12 = -b;
            w = (s16)-b >> 5;
        }
    blk17:
        m->spd = w;
    blk18:
        if (func_80072A00(o, dir) == 0) {
            m->w12 = 0;
            m->spd = 0;
            m->cnt++;
            if (m->cnt >= 0x42) {
                func_801C5CE0(o, 0, 0);
            }
        } else {
            x = (((u8 *)o - D_801D0B68) >> 6) + 0x3000;
            m->cnt = 0;
            func_8004CC30(o->def->snd8, x, (u8 *)o + 0x10, 0x64, 8000, 10000, abs(m->spd));
        }
    }
}
