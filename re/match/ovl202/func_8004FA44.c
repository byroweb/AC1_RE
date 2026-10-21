#include "ac1/ac1.h"

extern long SquareRoot0(long a);

/* Clip the segment (x0,y0)-(x1,y1) to the circle x*x + y*y <= r2 (fixed point,
 * slopes in 1/64).  Returns 0 when the line misses the circle. */
s32 func_8004FA44(s32 x0, s32 y0, s32 x1, s32 y1, s32 r2, s32 *ox0, s32 *oy0, s32 *ox1, s32 *oy1) {
    s32 dx;
    s32 dy;
    s32 k;
    s32 b;
    s32 kb2;
    s32 den;
    s32 disc;
    s32 sq;
    s32 tmp;
    s32 adx;
    s32 ady;
    s32 ax;
    s32 ay;
    s32 bx;
    s32 by;

    dx = x1 - x0;
    adx = abs(dx);
    dy = y1 - y0;
    ady = abs(dy);
    if (ady < adx) {
    xmajor:
        k = (dy << 6) / dx;
        b = ((-k * x0) >> 6) + y0;
        den = k * k + 4096;
        kb2 = k * b * 2;
        tmp = b * b - r2;
        disc = (kb2 >> 6) * (kb2 >> 6) - (den >> 4) * (tmp >> 6);
        if (disc < 0) {
            return 0;
        }
        sq = SquareRoot0(disc) << 6;
        ax = ((-kb2 + sq) << 5) / den;
        bx = ((-kb2 - sq) << 5) / den;
        if (dx < 0) {
            tmp = x0; x0 = x1; x1 = tmp;
            tmp = y0; y0 = y1; y1 = tmp;
        }
        if (ax < x0) {
            ax = x0; ay = y0;
        } else if (x1 < ax) {
            ax = x1; ay = y1;
        } else {
            ay = ((k * ax) >> 6) + b;
        }
        if (bx < x0) {
            bx = x0; by = y0;
        } else if (x1 < bx) {
            bx = x1; by = y1;
        } else {
            by = ((k * bx) >> 6) + b;
        }
    } else if (adx < ady) {
        k = (dx << 6) / dy;
        b = ((-k * y0) >> 6) + x0;
        den = k * k + 4096;
        kb2 = k * b * 2;
        tmp = b * b - r2;
        disc = (kb2 >> 6) * (kb2 >> 6) - (den >> 4) * (tmp >> 6);
        if (disc < 0) {
            return 0;
        }
        sq = SquareRoot0(disc) << 6;
        ay = ((-kb2 + sq) << 5) / den;
        by = ((-kb2 - sq) << 5) / den;
        if (dy < 0) {
            tmp = y0; y0 = y1; y1 = tmp;
            tmp = x0; x0 = x1; x1 = tmp;
        }
        if (ay < y0) {
            ax = x0; ay = y0;
        } else if (y1 < ay) {
            ax = x1; ay = y1;
        } else {
            ax = ((k * ay) >> 6) + b;
        }
        if (by < y0) {
            bx = x0; by = y0;
        } else if (y1 < by) {
            bx = x1; by = y1;
        } else {
            bx = ((k * by) >> 6) + b;
        }
    } else {
        if (dx != 0) {
            goto xmajor;
        }
        return 0;
    }
    *ox0 = ax;
    *oy0 = ay;
    *ox1 = bx;
    *oy1 = by;
    return 1;
}
