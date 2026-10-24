#include "ac1/ac1.h"
/* matched (wave4 a5). The v[0..3].vz stores go through SVECTOR views based at +4 so the v[1..3] base constants are not forced into s-registers; struct (in-struct) stores keep the a->vx load after them. */
/* ovl202 0x8005C710 - is the box min..max (any of its 8 corners) possibly on screen?
 * Corners go to scratchpad SVECTOR[8]; clip codes (func_8005C630) are AND-ed: 1 as soon
 * as the AND is 0, 0 when a corner is far behind the eye or all share an outcode. */

s32 func_8005C630(SVECTOR *v);

#define v ((SVECTOR *)0x1F800000)

s32 func_8005C710(SVECTOR *a, SVECTOR *b)
{
    s32 c;
    u32 f;
    SVECTOR *sv;

    sv = (SVECTOR *)0x1F800000;
    ((SVECTOR *)0x1F800004)->vx = ((SVECTOR *)0x1F80000C)->vx = ((SVECTOR *)0x1F800014)->vx = ((SVECTOR *)0x1F80001C)->vx = a->vz;
    v[4].vx = sv[0].vx = a->vx;
    v[4].vy = v[0].vy = a->vy;
    c = func_8005C630(&v[0]);
    if (c < 0) {
        return 0;
    }
    f = c;
    if (f == 0) {
        return 1;
    }
    v[5].vx = v[1].vx = b->vx;
    v[5].vy = v[1].vy = b->vy;
    c = func_8005C630(&v[1]);
    if (c < 0) {
        return 0;
    }
    f &= c;
    if (f == 0) {
        return 1;
    }
    v[6].vx = v[2].vx = a->vx;
    v[6].vy = v[2].vy = b->vy;
    c = func_8005C630(&v[2]);
    if (c < 0) {
        return 0;
    }
    f &= c;
    if (f == 0) {
        return 1;
    }
    v[7].vx = v[3].vx = b->vx;
    v[7].vy = v[3].vy = a->vy;
    c = func_8005C630(&v[3]);
    if (c < 0) {
        return 0;
    }
    f &= c;
    if (f == 0) {
        return 1;
    }
    v[4].vz = v[5].vz = v[6].vz = v[7].vz = b->vz;
    c = func_8005C630(&v[4]);
    if (c < 0) {
        return 0;
    }
    f &= c;
    if (f == 0) {
        return 1;
    }
    c = func_8005C630(&sv[5]);
    if (c < 0) {
        return 0;
    }
    f &= c;
    if (f == 0) {
        return 1;
    }
    c = func_8005C630(&sv[6]);
    if (c < 0) {
        return 0;
    }
    f &= c;
    if (f == 0) {
        return 1;
    }
    c = func_8005C630(&sv[7]);
    if (c < 0) {
        return 0;
    }
    f &= c;
    return f == 0;
}
