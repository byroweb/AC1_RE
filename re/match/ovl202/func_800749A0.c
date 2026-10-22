#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x0C];
    s32 kind;           /* +0x0C low 3 bits */
} HitDef;

typedef struct {
    HitDef *def;
} HitEnt;

typedef struct {
    u8 pad[4];
    void (*fn)(void *o, void *a, SVECTOR *v, SVECTOR *q, s32 b);
} HitCb;

typedef struct {
    u8 pad[0x14];
    HitCb *cb;          /* +0x14 */
    u8 pad18[4];
    u16 flags;          /* +0x1C */
} HitObj;

typedef struct {
    s32 flag;
    void *hit;
} HitWatch;

typedef struct {
    u8 pad[0x6A];
    u16 r;              /* +0x6A */
    u16 h;              /* +0x6C */
} AcSz;

typedef void (*MoveFn)(AcEntity *, s32);

extern SVECTOR D_80039C78;
extern u8 D_8008E518[];
extern u16 D_8008E51A;
extern s32 D_801A5DCC;
extern HitWatch D_801A5DD0;
extern HitEnt *func_80074800(AcEntity *e, SVECTOR *p, SVECTOR *v, s32 f);
extern HitObj *func_800733D8(AcEntity *e, SVECTOR *a, s32 b, s32 c, SVECTOR *d);
extern void *func_80074620(AcEntity *e, SVECTOR *a, s32 b, s32 c, SVECTOR *d);

void *func_800749A0(AcEntity *e, SVECTOR *pos, SVECTOR *out, SVECTOR *v)
{
    SVECTOR v0;
    SVECTOR q;
    void *hit;
    HitEnt *r;
    HitObj *o;
    void *r2;
    s32 tries;
    s32 k;

    hit = 0;
    tries = 6;
    do {
        v0 = *v;
        out->vx = pos->vx + v0.vx;
        out->vy = pos->vy + v0.vy;
        out->vz = pos->vz + v0.vz;
        r = func_80074800(e, out, v, 0);
        if (r == 0) {
            q.vx = pos->vx + v->vx;
            q.vy = pos->vy + v->vy;
            q.vz = pos->vz + v->vz;
            o = func_800733D8(0, &q, ((AcSz *)e)->r, ((AcSz *)e)->h, v);
            if (o != 0) {
                if (o->flags & 0x2000) {
                    if (o->cb != 0 && o->cb->fn != 0) {
                        o->cb->fn(o, D_8008E518, v, &q, D_8008E51A);
                    }
                } else {
                    hit = o;
                }
            }
            r2 = func_80074620(e, &q, ((AcSz *)e)->r, ((AcSz *)e)->h, v);
            if (r2 != 0) {
                hit = r2;
            }
            if (o == 0 && r2 == 0) {
                return hit;
            }
            if (D_801A5DCC == 1) {
                *v = D_80039C78;
                return hit;
            }
            if (v0.vx == v->vx && v0.vy == v->vy && v0.vz == v->vz) {
                break;
            }
        } else {
            hit = r;
            if (D_801A5DD0.flag != 0) {
                k = r->def->kind & 7;
                if (k == 2 || k == 3) {
                    D_801A5DD0.hit = hit;
                    if (e->fn_move != 0) {
                        ((MoveFn)e->fn_move)(e, 7);
                    }
                }
            }
        }
    } while (--tries);
    v->vz = 0;
    v->vy = 0;
    v->vx = 0;
    out->vx = pos->vx;
    out->vy = pos->vy;
    out->vz = pos->vz;
    return hit;
}
