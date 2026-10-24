/* MATCHED (wave4 a6): ax_b/ay_b = abs(car->pos.vx/vy) (draft negated uninitialised vars), ay_b declared before idx (spill-slot order), vy >= -15 arm first, s16 extra, final loop reloads h->e into f. */
/* ovl203 func_80071D84: like func_800718CC for a carrier that ACs can stand on (lift):
 * try the carried box first; an AC touching it is pushed, and the move is refused (return
 * 0, object left in place) when the push is shorter than half the carry; otherwise retry
 * the old box raised by the climb margin.  Pushed AC positions are committed at the end. */
#include "ac1/ac1.h"

typedef struct {
    SVECTOR lo;         /* +0x00 */
    SVECTOR hi;         /* +0x08 */
} V71DBox;

typedef struct {
    u8 pad0[8];
    SVECTOR pos;        /* +0x08 */
} V71DCarrier;

typedef struct {
    SVECTOR a;          /* +0x00 */
    SVECTOR b;          /* +0x08 */
    SVECTOR c;          /* +0x10 */
    V71DBox box;        /* +0x18 */
    u8 pad28[4];
    V71DCarrier *car;   /* +0x2C */
    u8 pad30[0x10];
} V71DObj;

typedef struct {
    u8 pad0[0x14];
    u16 flags;          /* +0x14 */
} V71DDef;

typedef struct {
    V71DDef *def;       /* +0x00 */
    u8 pad4[4];
    SVECTOR pos;        /* +0x08 */
    u8 pad10[0x28 - 0x10];
    u16 r;              /* +0x28 */
    u16 h;              /* +0x2A */
    u8 pad2c[0x170 - 0x2C];
} V71DEnt;

extern V71DObj D_801C40F0[];
extern V71DEnt D_8019B550[];
extern void func_8006D1A0(V71DObj *o, s32 idx);
extern int abs(int);
extern s32 func_800736CC(V71DEnt *e, SVECTOR *pos, SVECTOR *out, SVECTOR *push);
extern void func_8006D0DC(V71DObj *o, s32 idx);

typedef struct {
    V71DEnt *e;
    SVECTOR out;
} V71DHit;

s32 func_80071D84(V71DObj *o)
{
    s32 ay_b;
    s32 idx;
    V71DCarrier *car;
    SVECTOR c0;
    SVECTOR c1;
    V71DBox b0;
    V71DBox b1;
    V71DHit list[2];
    V71DBox hit;
    SVECTOR spare;
    SVECTOR push;
    V71DEnt *e;
    V71DHit *h;
    V71DEnt *f;
    s32 i;
    s32 ax;
    s32 ax_b;
    s32 t;
    s16 extra;
    s32 res;
    s16 dy;

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
    res = 1;
    dy = 0;
    ax_b = abs(car->pos.vx);
    ay_b = abs(car->pos.vy);
    if (car->pos.vy >= -15) {
        extra = car->pos.vy + 16;
    } else {
        extra = 16;
        dy = (car->pos.vy >> 1) - 8;
    }
    h = list;
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
        hit.lo.vy = e->pos.vy - e->h - 50;
        hit.hi.vy = e->pos.vy + dy;
        o->c = c1;
        o->box = b1;
        if (func_80071638(o, &hit, 0, 0) != 0) {
            push = car->pos;
            e->h += 50;
            t = func_800736CC(e, &e->pos, &h->out, &push);
            e->h -= 50;
            if (t == 0) {
                h->e = e;
                h++;
                continue;
            }
            if (abs(push.vx) * 2 < ax_b || abs(push.vy) * 2 < ay_b || abs(push.vz) * 2 < ax_b) {
            fail:
                res = 0;
                o->c = c0;
                o->box = b0;
                goto done;
            }
            if ((car->pos.vy > 0 && push.vy < 0) || (car->pos.vy < 0 && push.vy > 0))
                goto fail;
            h->e = e;
            h++;
            continue;
        }
        o->c = c0;
        o->box = b0;
        hit.hi.vy += extra;
        if (func_80071638(o, &hit, 0, 0) != 0) {
            push = car->pos;
            func_800736CC(e, &e->pos, &h->out, &push);
            h->e = e;
            h++;
        }
    }
    h->e = 0;
    for (h = list; h->e != 0; h++) {
        f = h->e;
        f->pos.vx = h->out.vx;
        f->pos.vy = h->out.vy;
        f->pos.vz = h->out.vz;
    }
    o->c = c1;
    o->box = b1;
    o->a.vx += car->pos.vx;
    o->a.vy += car->pos.vy;
    o->a.vz += car->pos.vz;
    o->b.vx += car->pos.vx;
    o->b.vy += car->pos.vy;
    o->b.vz += car->pos.vz;
done:
    func_8006D0DC(o, idx);
    return res;
}
