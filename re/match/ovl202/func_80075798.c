#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x28];
    u16 h28;            /* +0x28 horizontal step limit */
    u16 h2a;            /* +0x2A height */
} AcSize;

extern void VectorNormalSS(SVECTOR *v0, SVECTOR *v1);
extern s32 ratan2(s32 y, s32 x);

s32 func_80075798(AcEntity *e, u8 *def, SVECTOR *dir, SVECTOR *tgt, s32 speed, SVECTOR *out)
{
    SVECTOR n2;
    SVECTOR d;
    SVECTOR n1;
    s32 k;
    s32 ang;
    s32 x;
    s32 y;
    s32 z;
    s32 lim;
    s32 h;

    k = ((speed << 3) * def[4]) >> 12;
    d.vx = e->pos.x - tgt->vx;
    d.vy = 0;
    d.vz = e->pos.z - tgt->vz;
    VectorNormalSS(&d, &n1);
    VectorNormalSS(dir, &n2);
    n2.vz += n1.vz >> 4;
    n2.vx += n1.vx >> 4;
    ang = 0x800 - (e->yaw - ratan2(n2.vz, n2.vx));
    n2.vx = (n2.vx * k) >> 13;
    n2.vy = (n2.vy * k) >> 12;
    n2.vz = (n2.vz * k) >> 13;
    x = n2.vx + e->pos.x - e->prev_pos.x;
    lim = ((AcSize *)e)->h28;
    if (x > 0) {
        if (x > lim) {
            x = lim;
        }
    } else {
        if (x < -lim) {
            x = -lim;
        }
    }
    z = n2.vz + e->pos.z - e->prev_pos.z;
    if (z > 0) {
        if (z > lim) {
            z = lim;
        }
    } else {
        if (z < -lim) {
            z = -lim;
        }
    }
    y = n2.vy + e->pos.y - e->prev_pos.y;
    h = ((AcSize *)e)->h2a >> 1;
    if (y > 0) {
        if (y > h) {
            y = h;
        }
    } else {
        h = -h;
        if (y < h) {
            y = h;
        }
    }
    out->vx = x;
    out->vy = y;
    out->vz = z;
    return ang;
}
