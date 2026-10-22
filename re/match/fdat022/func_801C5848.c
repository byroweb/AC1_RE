#include "ac1/ac1.h"

typedef struct {
    u8 pad0[8];
    s16 f8;
} Sub;
typedef struct {
    u8 pad0[4];
    Sub *sub;           /* +0x04 */
    u8 pad8[2];
    s16 fa;             /* +0x0A */
    u8 padc[6];
    s16 ang;            /* +0x12 */
    u8 pad14[0x16];
    u16 f2a;            /* +0x2A */
    u8 pad2c[2];
    s16 f2e;            /* +0x2E */
    u8 pad30[0xC];
    u8 mode;            /* +0x3C */
    u8 pad3d[4];
    u8 state;           /* +0x41 */
    u8 pad42[0xE];
    void (*fn)(void *, u8);     /* +0x50 */
} Ent;
extern s16 func_800132CC();
extern void func_80014338();
extern void func_800143B8();
extern void func_80014728();
extern void func_8004C98C();
extern s32 func_8004F080();
extern s16 func_80075EB4();
extern void func_80078250();
extern s32 rand();
extern s32 D_801A5DC0;

void func_801C5848(Ent *e)
{
    s16 sp28[4];
    s16 a0;
    s16 v0;
    s16 a1;
    u8 *p;

    a0 = e->f2e;
    v0 = e->fa;
    if (a0 <= v0 && a0 >= v0 - e->f2a) {
        func_80078250(e, 0, 2, 0, 0x8C, (rand() & 0x7F) + 0xFA, 0x42, D_801A5DC0 + 0x1F00);
    }
    a1 = func_80075EB4(e);
    switch (e->mode) {
    case 1:
    calc:
        v0 = func_800132CC(e->ang, a1, e->sub->f8);
        e->ang = v0;
        func_80014728(v0, sp28);
        func_80014338(sp28, 0xFA0, (u8 *)e + 0xCC);
        break;
    case 2:
        a1 += 0x200;
        goto calc;
    case 3:
        a1 -= 0x200;
        goto calc;
    default:
        p = (u8 *)e + 0xCC;
        func_800143B8(p, 0x7D0, 0xFA0, p);
        break;
    }
    func_8004C98C(0x5C, D_801A5DC0 + 0x1300, (u8 *)e + 8, (u8 *)e + 0xCC, 0x64, 0x1B58, 0x1F40, 0, 0x50);
    if (func_8004F080((u8 *)e + 0x64, (u8 *)e + 8) == 0) {
        if (e->fn != 0) {
            e->fn(e, e->state);
        }
    }
}
