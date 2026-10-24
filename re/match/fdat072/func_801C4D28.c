#include "ac1/ac1.h"

typedef struct {
    u8 pad0[8];
    s16 f8;
} Sub;
typedef struct {
    u8 pad0[4];
    Sub *sub;           /* +0x04 */
    u8 pad8[0xA];
    s16 ang;            /* +0x12 */
    u8 pad14[0x28];
    u8 mode;            /* +0x3C */
    u8 pad3d[4];
    u8 state;           /* +0x41 */
    u8 pad42[0xE];
    void (*fn)(void *, u8);     /* +0x50 */
    u8 pad54[0xB4];
    s16 spd;            /* +0x108 */
} Ent;
extern s16 func_800132CC();
extern s16 func_80013390();
extern void func_80014338();
extern s32 func_800143B8();
extern void func_80014728();
extern void func_8004CC30();
extern s32 func_8004F080();
extern s16 func_80075EB4();
extern struct { s32 v; } D_801A5DC0;

void func_801C4D28(Ent *e)
{
    s16 sp20[4];
    s32 s0;
    s16 a1;
    s32 a0;
    s16 b0;
    s16 v;
    s32 t;
    s32 v2;
    u8 *p;
    u8 *q;

    s0 = 0;
    a1 = func_80075EB4();
    switch (e->mode) {
    case 1:
    body:
        v = func_800132CC(e->ang, a1, e->sub->f8);
        e->ang = v;
        if (s0 != 0) {
            func_80014728((v + 0x800) % 0x1000, sp20);
        } else {
            func_80014728(v, sp20);
        }
        func_80014338(sp20, 1000, (u8 *)e + 0xCC);
        v2 = 1000;
        break;
    case 2:
        a1 += 0x200;
        goto body;
    case 3:
        a1 -= 0x200;
        goto body;
    case 6:
        a1 += 0x800;
        goto body;
    case 7:
        a1 += 0x600;
        goto body;
    case 8:
        a1 += 0xA00;
        goto body;
    case 4:
        a1 += 0x400;
        goto body;
    case 5:
        a1 -= 0x400;
        goto body;
    case 12:
        a1 += 0x200;
    case 11:
    one:
        s0 = 1;
        goto body;
    case 13:
        a1 -= 0x200;
        goto one;
    default:
        q = (u8 *)e + 0xCC;
        v2 = func_800143B8(q, 500, 1000, q);
        break;
    }
    b0 = func_80013390(e->spd, v2, 0x32);
    p = (u8 *)e + 8;
    e->spd = b0;
    func_8004CC30(0xA1, D_801A5DC0.v + 0x1300, p, (b0 >> 4) + 0x32, 8000, 10000, (s32)((u16)e->spd << 16) >> 21);
    if (func_8004F080((u8 *)e + 0x64, p) == 0) {
        if (e->fn != 0) {
            e->fn(e, e->state);
        }
    }
}
