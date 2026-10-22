/* ovl203 func_800718CC: move a 64-byte attached object (table at 0x801C40F0) with its
 * carrier: try its box at the old and at the carried position against both versus ACs
 * (func_80071638); an AC that touches it is pushed (func_800736CC).  Finally commit the
 * carried position and refresh the object (func_8006D0DC). */
#include "ac1/ac1.h"

typedef struct {
    SVECTOR lo;         /* +0x00 */
    SVECTOR hi;         /* +0x08 */
} V718Box;

typedef struct {
    u8 pad0[8];
    SVECTOR pos;        /* +0x08 */
} V718Carrier;

typedef struct {
    SVECTOR a;          /* +0x00 */
    SVECTOR b;          /* +0x08 */
    SVECTOR c;          /* +0x10 */
    V718Box box;        /* +0x18 */
    u8 pad28[4];
    V718Carrier *car;   /* +0x2C */
    u8 pad30[0x10];
} V718Obj;

typedef struct {
    u8 pad0[0x14];
    u16 flags;          /* +0x14 */
} V718Def;

typedef struct {
    V718Def *def;       /* +0x00 */
    u8 pad4[4];
    SVECTOR pos;        /* +0x08 */
    u8 pad10[0x28 - 0x10];
    u16 r;              /* +0x28 */
    u16 h;              /* +0x2A */
    u8 pad2c[0x170 - 0x2C];
} V718Ent;

extern V718Obj D_801C40F0[];
extern V718Ent D_8019B550[];
extern void func_8006D1A0(V718Obj *o, s32 idx);
extern s32 func_80071638(V718Obj *o, V718Box *b, s32 a, s32 c);
extern void func_800736CC(V718Ent *e, SVECTOR *pos, SVECTOR *out, SVECTOR *push);
extern void func_8006D0DC(V718Obj *o, s32 idx);

void func_800718CC(V718Obj *o)
{
    s32 idx;
    V718Carrier *car;
    SVECTOR c0;
    SVECTOR c1;
    V718Box b0;
    V718Box b1;
    V718Box hit;
    SVECTOR out;
    SVECTOR push;
    V718Ent *e;
    s32 i;

    idx = o - D_801C40F0;
    car = o->car;
    func_8006D1A0(o, idx);
    c0 = o->c;
    c1 = c0;
    c1.vx += car->pos.vx;
    c1.vy += car->pos.vy;
    c1.vz += car->pos.vz;
    b0 = o->box;
    b1 = b0;
    b1.lo.vx += car->pos.vx;
    b1.hi.vx += car->pos.vx;
    b1.lo.vy += car->pos.vy;
    b1.hi.vy += car->pos.vy;
    b1.lo.vz += car->pos.vz;
    b1.hi.vz += car->pos.vz;
    e = D_8019B550;
    for (i = 2; i != 0; e++, i--) {
        if (e->def == 0)
            continue;
        if (e->r == 0)
            continue;
        if (e->def->flags & 4)
            continue;
        hit.lo.vx = e->pos.vx - e->r;
        hit.hi.vx = e->pos.vx + e->r;
        hit.lo.vz = e->pos.vz - e->r;
        hit.hi.vz = e->pos.vz + e->r;
        hit.lo.vy = e->pos.vy - e->h;
        hit.hi.vy = e->pos.vy + 24;
        o->c = c0;
        o->box = b0;
        if (func_80071638(o, &hit, 0, 0) != 0) {
        push_it:
            push = car->pos;
            func_800736CC(e, &e->pos, &out, &push);
            e->pos.vx = out.vx;
            e->pos.vy = out.vy;
            e->pos.vz = out.vz;
        } else {
            o->c = c1;
            o->box = b1;
            if (func_80071638(o, &hit, 0, 0) != 0)
                goto push_it;
        }
    }
    o->c = c1;
    o->box = b1;
    o->a.vx += car->pos.vx;
    o->a.vy += car->pos.vy;
    o->a.vz += car->pos.vz;
    o->b.vx += car->pos.vx;
    o->b.vy += car->pos.vy;
    o->b.vz += car->pos.vz;
    func_8006D0DC(o, idx);
}
