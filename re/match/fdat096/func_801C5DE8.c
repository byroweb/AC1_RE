#include "ac1/ac1.h"

typedef struct { char b[8]; } Blk8;
typedef struct {
    u8 pad0[8];
    Blk8 v;             /* +0x08 */
    s16 pos[3];         /* +0x10 */
    s16 a[3];           /* +0x16? */
    u8 pad1c[0x10];
} Eff;
extern s32 func_80072a00();
extern u8 D_80039C78[];

typedef struct {
    s16 lim[3];         /* +0x00 */
    s16 vel[3];         /* +0x06 */
} Def;
typedef struct {
    u8 pad0[8];
    Blk8 v;             /* +0x08 */
    s16 pos[3];         /* +0x10 */
    s16 pad16;
    s16 lo[3];          /* +0x18 */
    s16 hi[3];          /* +0x1E */
} E2;
typedef struct {
    u8 pad0[0x10];
    s16 pos[3];         /* +0x10 */
    u8 pad16[0x16];
    E2 *e;              /* +0x2C */
    u8 pad30[8];
    Def *def;           /* +0x38 */
} Obj;

void func_801C5DE8(Obj *o)
{
    E2 *p;
    Def *d;
    s16 v;
    s32 c;
    s16 nx0, nx1, nx2;

    p = o->e;
    d = o->def;
    p->pos[0] = p->pos[0] + d->vel[0];
    if (d->vel[0] > 0) {
        if (p->pos[0] > d->lim[0]) {
            p->pos[0] = d->lim[0];
        }
    } else {
        if (p->pos[0] < -d->lim[0]) {
            p->pos[0] = -d->lim[0];
        }
    }
    p->pos[1] = p->pos[1] + d->vel[1];
    if (d->vel[1] > 0) {
        if (p->pos[1] > d->lim[1]) {
            p->pos[1] = d->lim[1];
        }
    } else {
        if (p->pos[1] < -d->lim[1]) {
            p->pos[1] = -d->lim[1];
        }
    }
    p->pos[2] = p->pos[2] + d->vel[2];
    if (d->vel[2] > 0) {
        if (p->pos[2] > d->lim[2]) {
            p->pos[2] = d->lim[2];
        }
    } else {
        if (p->pos[2] < -d->lim[2]) {
            p->pos[2] = -d->lim[2];
        }
    }
    p->v = *(Blk8 *)p->pos;
    if (func_80072a00(o) == 0) {
        p->v = *(Blk8 *)D_80039C78;
    }
    if (d->vel[0] > 0) {
        if (o->pos[0] > p->hi[0]) {
            d->vel[0] = -d->vel[0];
        }
    } else if (o->pos[0] < p->lo[0]) {
        d->vel[0] = -d->vel[0];
    }
    if (d->vel[1] > 0) {
        if (o->pos[1] > p->hi[1]) {
            d->vel[1] = -d->vel[1];
        }
    } else if (o->pos[1] < p->lo[1]) {
        d->vel[1] = -d->vel[1];
    }
    if (d->vel[2] > 0) {
        if (o->pos[2] > p->hi[2]) {
            d->vel[2] = -d->vel[2];
        }
    } else if (o->pos[2] < p->lo[2]) {
        d->vel[2] = -d->vel[2];
    }
}
