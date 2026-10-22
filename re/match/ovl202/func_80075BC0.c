#include "ac1/ac1.h"

typedef struct {
    u8 pad[0xDC];
    u8 mode;            /* +0xDC */
    u8 paddd[3];
    AcEntity *tgt;      /* +0xE0 */
    SVECTOR off;        /* +0xE4 */
} AcAim;

#define A(e) ((AcAim *)(e))

extern SVECTOR D_80039C78;
extern void func_8001380C(SVECTOR *in, SVECTOR *out, s32 n);

void func_80075BC0(AcEntity *e, SVECTOR *out)
{
    SVECTOR d;
    AcEntity *t;
    s32 y;

    if (A(e)->mode < 4) {
        t = A(e)->tgt;
        if (t == 0 || t->def == 0) {
            goto zero;
        }
        d.vx = t->pos.x + A(e)->off.vx - e->pos.x;
        d.vy = t->pos.y + A(e)->off.vy - e->pos.y;
        d.vz = t->pos.z + A(e)->off.vz - e->pos.z;
        switch (A(e)->mode) {
        case 0:
            *out = d;
            return;
        case 1:
            out->vx = -d.vx;
            out->vy = -d.vy;
            out->vz = -d.vz;
            return;
        case 2:
            func_8001380C(&d, out, 1);
            return;
        case 3:
            func_8001380C(&d, out, 3);
            return;
        }
        goto zero;
    }
    if (A(e)->mode < 6) {
        switch (A(e)->mode) {
        case 4:
            t = A(e)->tgt;
            if (t == 0 || t->def == 0) {
                goto zero;
            }
            out->vx = t->pos.x + A(e)->off.vx - e->pos.x;
            out->vz = t->pos.z + A(e)->off.vz - e->pos.z;
            y = A(e)->off.vy - e->pos.y;
            if (y > 32767) {
                out->vy = 32767;
            } else if (y < -32768) {
                out->vy = -32768;
            } else {
                out->vy = y;
            }
            return;
        case 5:
            t = A(e)->tgt;
            if (t == 0 || t->def == 0) {
                goto zero;
            }
            out->vx = e->pos.x - (t->pos.x + A(e)->off.vx);
            out->vz = e->pos.z - (t->pos.z + A(e)->off.vz);
            y = A(e)->off.vy - e->pos.y;
            if (y > 32767) {
                out->vy = 32767;
            } else if (y < -32768) {
                out->vy = -32768;
            } else {
                out->vy = y;
            }
            return;
        }
    } else if (A(e)->mode == 6) {
        out->vx = A(e)->off.vx - e->pos.x;
        out->vz = A(e)->off.vz - e->pos.z;
        y = A(e)->off.vy - e->pos.y;
        if (y > 32767) {
            out->vy = 32767;
        } else if (y < -32768) {
            out->vy = -32768;
        } else {
            out->vy = y;
        }
        return;
    }
zero:
    *out = D_80039C78;
}
