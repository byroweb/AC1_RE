/* exe: intersect the line through (x0,y0) with direction (dx,dy) and the
 * circle x*x + y*y = r2 (fixed point, slopes in 1/64).  Solves along the
 * longer axis; returns 1 (solved for x) or 2 (solved for y) with the
 * nearer in-segment point in *ox/*oy, 0 when there is none. */
#include "ac1/ac1.h"

extern s32 SquareRoot0(s32 a);
int abs(int);

s32 func_80012A78(s32 x0, s32 y0, s32 dx, s32 dy, s32 r2, s32 *ox, s32 *oy)
{
    s32 adx;
    s32 ady;
    s32 k;
    s32 c;
    s32 b2;
    s32 a;
    s32 disc;
    s32 s;
    s32 p1;
    s32 p2;
    s32 d1;
    s32 d2;
    s32 f;
    s32 t;

    adx = abs(dx);
    ady = abs(dy);
    if (ady < adx) {
    alongx:
        k = (dy << 6) / dx;
        c = ((-k * x0) >> 6) + y0;
        a = k * k + 4096;
        b2 = k * c * 2;
        t = c * c - r2;
        disc = (b2 >> 6) * (b2 >> 6) - (a >> 4) * (t >> 6);
        if (disc < 0) {
            return 0;
        }
        s = SquareRoot0(disc) << 6;
        p1 = ((-b2 + s) << 5) / a;
        f = 0;
        p2 = ((-b2 - s) << 5) / a;
        d1 = p1 - x0;
        if (dx >= 0) {
            if (d1 < 0 || dx < d1) {
                f = 1;
            }
        } else if (d1 > 0 || d1 < dx) {
            f = 1;
        } else {
            d1 = -d1;
        }
        d2 = p2 - x0;
        if (dx > 0) {
            if (d2 < 0 || dx < d2) {
                f |= 2;
            }
        } else if (d2 > 0 || d2 < dx) {
            f |= 2;
        } else {
            d2 = -d2;
        }
        switch (f) {
        case 0:
            if (d2 >= d1) {
                break;
            }
        case 1:
            p1 = p2;
            break;
        case 2:
            break;
        case 3:
            return 0;
        }
        *ox = p1;
        *oy = ((k * p1) >> 6) + c;
        return 1;
    }
    if (adx < ady) {
        s32 q1;
        s32 q2;
        s32 e1;
        s32 e2;

        k = (dx << 6) / dy;
        c = ((-k * y0) >> 6) + x0;
        a = k * k + 4096;
        b2 = k * c * 2;
        t = c * c - r2;
        disc = (b2 >> 6) * (b2 >> 6) - (a >> 4) * (t >> 6);
        if (disc < 0) {
            return 0;
        }
        s = SquareRoot0(disc) << 6;
        q1 = ((-b2 + s) << 5) / a;
        f = 0;
        q2 = ((-b2 - s) << 5) / a;
        e1 = q1 - y0;
        if (dy >= 0) {
            if (e1 < 0 || dy < e1) {
                f = 1;
            }
        } else if (e1 > 0 || e1 < dy) {
            f = 1;
        } else {
            e1 = -e1;
        }
        e2 = q2 - y0;
        if (dy > 0) {
            if (e2 < 0 || dy < e2) {
                f |= 2;
            }
        } else if (e2 > 0 || e2 < dy) {
            f |= 2;
        } else {
            e2 = -e2;
        }
        switch (f) {
        case 0:
            if (e2 >= e1) {
                break;
            }
        case 1:
            q1 = q2;
            break;
        case 2:
            break;
        case 3:
            return 0;
        }
        *oy = q1;
        *ox = ((k * q1) >> 6) + c;
        return 2;
    }
    if (dx != 0) {
        goto alongx;
    }
    return 0;
}
