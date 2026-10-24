/* collision of a moving point with a capped vertical cylinder */
#include "ac1/ac1.h"

extern s32 func_80012A78(s32 x, s32 z, s16 dx, s16 dz, s32 r2, s32 *ox, s32 *oz);

s32 func_80012E5C(SVECTOR *p, SVECTOR *d, SVECTOR *out, SVECTOR *c, s32 r, s32 h)
{
    s32 lo, hi;
    s32 cmin, cmax;
    s32 r2;
    s32 ex, ey, ez;
    s32 ax, az;
    s32 v;
    s32 tx, tz;

    if (d->vx >= 0) {
        lo = p->vx;
        hi = lo + d->vx;
    } else {
        hi = p->vx;
        lo = hi + d->vx;
    }
    cmin = c->vx - r;
    cmax = c->vx + r;
    if ((lo < cmin) ? (hi < cmin) : (cmax < lo)) {
        return 0;
    }
    if (d->vz >= 0) {
        lo = p->vz;
        hi = lo + d->vz;
    } else {
        hi = p->vz;
        lo = hi + d->vz;
    }
    cmin = c->vz - r;
    cmax = c->vz + r;
    if ((lo < cmin) ? (hi < cmin) : (cmax < lo)) {
        return 0;
    }
    if (d->vy >= 0) {
        lo = p->vy;
        hi = lo + d->vy;
    } else {
        hi = p->vy;
        lo = hi + d->vy;
    }
    cmin = c->vy - h;
    cmax = c->vy;
    if ((lo < cmin) ? (hi < cmin) : (cmax < lo)) {
        return 0;
    }
    r2 = r * r;
    ex = c->vx - p->vx;
    ez = c->vz - p->vz;
    if (d->vy > 0) {
        ey = c->vy - p->vy;
slope:
        ax = (d->vx * ey / d->vy + ex) >> 2;
        az = (d->vz * ey / d->vy + ez) >> 2;
        if (ax * ax + az * az <= (r2 >> 4)) {
            out->vx = p->vx + ax;
            out->vy = p->vy + ey;
            out->vz = p->vz + az;
            return 1;
        }
        if (func_80012A78(-ex, -ez, d->vx, d->vz, r2, &tx, &tz) == 0) {
fail:
            return 0;
        }
        out->vx = c->vx + tx;
        out->vz = c->vz + tz;
        ex = out->vx - p->vx;
        ez = out->vz - p->vz;
        if (__builtin_abs(ex) >= __builtin_abs(ez)) {
            out->vy = p->vy + d->vy * ex / d->vx;
        } else {
            out->vy = p->vy + d->vy * ez / d->vz;
        }
        return 1;
    }
    if (d->vy < 0) {
        ey = c->vy - h - p->vy;
        goto slope;
    }
    if (func_80012A78(-ex, -ez, d->vx, d->vz, r2, &tx, &tz) == 0) {
        goto fail;
    }
    out->vy = p->vy;
    out->vx = c->vx + tx;
    out->vz = c->vz + tz;
    return 1;
}
