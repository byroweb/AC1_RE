#include "ac1/ac1.h"
/* ovl202 0x8004E06C - scale a step vector so no axis passes the limits:
 * factor = min(lim/|vx|, lim/|vz|, limy/|vy|). Factor 0: copy the vector and
 * clamp each zero-factor axis to +-limit, return 1. Else scale by
 * min(factor, max) and return it. */

s32 func_8004E06C(SVECTOR *v, SVECTOR *out, s32 lim, s32 limy, s32 max)
{
    s32 rx;
    s32 rz;
    s32 ry;
    s32 m;

    if (v->vx == 0) {
        rx = 0xFFFF;
    } else {
        rx = lim / v->vx;
        if (rx < 0) {
            rx = -rx;
        }
    }
    rz = 0xFFFF;
    if (v->vz != 0) {
        rz = lim / v->vz;
        if (rz < 0) {
            rz = -rz;
        }
    }
    ry = 0xFFFF;
    if (v->vy != 0) {
        ry = limy / v->vy;
        if (ry < 0) {
            ry = -ry;
        }
    }
    if (rx < rz) {
        m = rx;
    } else {
        m = rz;
    }
    if (ry < m) {
        m = ry;
    }
    if (m == 0) {
        *out = *v;
        if (rx == 0) {
            if (out->vx > 0) {
                out->vx = lim;
            } else if (out->vx < 0) {
                out->vx = -lim;
            } else {
                out->vx = 0;
            }
        }
        if (rz == 0) {
            if (out->vz > 0) {
                out->vz = lim;
            } else if (out->vz < 0) {
                out->vz = -lim;
            } else {
                out->vz = 0;
            }
        }
        if (ry == 0) {
            if (out->vy > 0) {
                out->vy = limy;
            } else if (out->vy < 0) {
                out->vy = -limy;
            } else {
                out->vy = 0;
            }
        }
        m = 1;
    } else {
        if (max < m) {
            m = max;
        }
        out->vx = v->vx * m;
        out->vy = v->vy * m;
        out->vz = v->vz * m;
    }
    return m;
}
