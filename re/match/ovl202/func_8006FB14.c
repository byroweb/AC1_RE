#include "ac1/ac1.h"
/* ovl202 0x8006FB14 - order a box's x and z bounds (min SVECTOR at 0, max at 8) */

void func_8006FB14(SVECTOR *b)
{
    s16 lo, hi, lo2, hi2;
    s16 t0, t1, t2, t3;

    lo = b[0].vx;
    hi = b[1].vx;
    if (lo > hi) {
        t0 = hi;
        t1 = lo;
        b[0].vx = t0;
        b[1].vx = t1;
    }
    lo2 = b[0].vz;
    hi2 = b[1].vz;
    if (lo2 > hi2) {
        t2 = hi2;
        t3 = lo2;
        b[0].vz = t2;
        b[1].vz = t3;
    }
}
