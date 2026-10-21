#include "ac1/ac1.h"

typedef struct {
    u16 len;            /* +0x00 */
    u8 pad2[5];
    u8 dir;             /* +0x07 */
} Func801C6020Def;
typedef struct {
    void (*fn)();       /* +0x00 */
    void (*msg)();      /* +0x04 */
    u8 pad8[8];
    u8 dir;             /* +0x10 */
    u8 pad11;
    s16 on;             /* +0x12 */
    u16 len;            /* +0x14 */
    s16 a;              /* +0x16 */
    s16 b;              /* +0x18 */
    s16 t;              /* +0x1A */
} Func801C6020Mover;
typedef struct {
    u8 pad0[0x2C];
    Func801C6020Mover *mover; /* +0x2C */
    u8 pad30[8];
    Func801C6020Def *def; /* +0x38 */
} Func801C6020Obj;
extern Func801C6020Mover *func_80072120(s32);
extern void func_801C5E5C();
extern void func_801C5D80();

void func_801C6020(Func801C6020Obj *o)
{
    u8 unused[144];
    Func801C6020Mover *m;
    u16 len;

    m = func_80072120(12);
    o->mover = m;
    m->fn = func_801C5E5C;
    m->msg = func_801C5D80;
    m->dir = o->def->dir;
    m->on = 0;
    m->a = o->def->len << 5;
    len = o->def->len;
    m->t = 0;
    m->len = len;
    m->b = (s16)len * 17;
}
