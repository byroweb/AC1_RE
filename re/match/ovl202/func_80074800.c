#include "ac1/ac1.h"

typedef struct {
    SVECTOR lo;
    SVECTOR hi;
} Box16;

typedef struct {
    u8 pad[0x28];
    u16 rad;            /* +0x28 */
    u16 h2a;            /* +0x2A */
    u8 pad2c[0x0E];
    u8 kind;            /* +0x3A */
} ColProbe;

extern s32 D_801A5DC8;
extern void *func_80070F30(Box16 *box, SVECTOR *d, SVECTOR *out, s32 a, s32 kind);

#define QUP(x) ((x) >= 0 ? (u32)((x) + 3) >> 2 : -((-(x) + 3) >> 2))

typedef struct {
    u16 vx;
    u16 vy;
    u16 vz;
    u16 pad;
} UVEC;

void *func_80074800(ColProbe *o, SVECTOR *p, UVEC *d, s32 a)
{
    Box16 box;
    SVECTOR dd;
    SVECTOR out;
    void *r;
    s32 t;

    box.lo.vx = p->vx - (o->rad >> D_801A5DC8);
    box.hi.vx = p->vx + (o->rad >> D_801A5DC8);
    box.lo.vz = p->vz - (o->rad >> D_801A5DC8);
    box.hi.vz = p->vz + (o->rad >> D_801A5DC8);
    box.lo.vy = p->vy - o->h2a;
    box.hi.vy = p->vy;
    if (d != 0) {
        dd.vx = d->vx * 4;
        dd.vy = d->vy * 4;
        dd.vz = d->vz * 4;
        r = func_80070F30(&box, &dd, &out, a, o->kind);
        if (r != 0) {
            t = out.vx;
            d->vx = QUP(t);
            t = out.vy;
            d->vy = QUP(t);
                    t = out.vz;
        if (t >= 0) {
            t = (u32)(t + 3) >> 2;
        } else {
            t = -((-t + 3) >> 2);
        }
        d->vz = t;
        }
    } else {
        r = func_80070F30(&box, 0, 0, a, o->kind);
    }
    return r;
}
