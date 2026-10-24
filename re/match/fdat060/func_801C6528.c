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
    u8 pad54[0x10];
    u8 pad64[8];        /* +0x64 */
    u8 padCC[1];
} Ent;
extern s16 func_800132CC();
extern void func_80014338();
extern void func_800143B8();
extern void func_80014728();
extern s32 func_8004F080();
extern s16 func_80075EB4();

void func_801C6528(Ent *e)
{
    s32 s1;
    s16 a1;
    s16 b0;
    s16 v;
    s32 t;
    s16 sp[4];
    u8 *p;

    s1 = 0;
    a1 = func_80075EB4();
    switch (e->mode) {
    case 1:
    body:
        v = func_800132CC(e->ang, a1, e->sub->f8);
        e->ang = v;
        if (s1 != 0) {
            func_80014728((v + 0x800) % 0x1000, sp);
        } else {
            func_80014728(v, sp);
        }
        func_80014338(sp, (s1 + 1) * 300, (u8 *)e + 0xCC);
        break;
    case 2:
        a1 += 0x200;
        goto body;
    case 3:
        a1 -= 0x200;
        goto body;
    case 7:
        a1 += 0x200;
    case 6:
    six:
        s1 = 1;
        goto body;
    case 8:
        a1 -= 0x200;
        goto six;
    case 4:
        a1 += 0x400;
        goto body;
    case 5:
        a1 -= 0x400;
        goto body;
    default:
        p = (u8 *)e + 0xCC;
        func_800143B8(p, 150, 300, p);
        break;
    }
    if (func_8004F080((u8 *)e + 0x64, (u8 *)e + 8) == 0) {
        if (e->fn != 0) {
            e->fn(e, e->state);
        }
    }
}
