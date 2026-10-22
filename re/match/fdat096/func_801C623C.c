#include "ac1/ac1.h"

typedef struct { char b[8]; } Blk8;
typedef struct {
    void *f0;           /* +0x00 */
    void *f4;           /* +0x04 */
    u8 pad8[8];
    s16 a, b, c;        /* +0x10 */
    u8 pad16[2];
    s16 d, e, f, g, h, i;   /* +0x18 */
} Eff;
typedef struct {
    s16 a, b, c;
    u8 pad6[6];
    u16 f0c;            /* +0x0C */
} Def;
typedef struct {
    s16 x, y, z;        /* +0 */
    u8 pad6[2];
    s16 u, v;           /* +8 */
    s16 w;              /* +0xC */
    u8 padE[0x1E];
    Eff *eff;           /* +0x2C */
    u8 pad30[8];
    Def *def;           /* +0x38 */
} Obj;
extern Eff *func_80072120();
extern void func_801C5DE8();
extern void func_801C605C();
extern u8 D_80039C78[];

void func_801C623C(Obj *o)
{
    char pad[144];
    Eff *p;

    p = func_80072120(0x14);
    o->eff = p;
    p->f0 = func_801C5DE8;
    p->a = o->def->a;
    {
        Def *d = o->def;
        d->a = d->a < 0 ? -d->a : d->a;
    }
    p->b = o->def->b;
    {
        Def *d = o->def;
        d->b = d->b < 0 ? -d->b : d->b;
    }
    p->c = o->def->c;
    {
        Def *d = o->def;
        d->c = d->c < 0 ? -d->c : d->c;
    }
    *(Blk8 *)&p->a = *(Blk8 *)D_80039C78;
    p->d = o->x;
    p->e = o->y;
    p->f = o->z;
    p->g = o->u;
    p->h = o->v;
    p->i = o->w;
    if (o->def->f0c != 0) {
        o->eff->f4 = func_801C605C;
    }
}
