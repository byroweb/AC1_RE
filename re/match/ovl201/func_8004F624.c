#include "ac1/ac1.h"
/* ovl201 0x8004F624 - step pos 8 units a frame towards (x, y, z) (the
 * direction taken from the sign of the target, or z < -2524); retire once
 * there. */

void func_8004F624(UiObj *o, s16 x, s16 y, s16 z)
{
    if (o->pos.vx != x) {
        if (x < 0) {
            o->pos.vx -= 8;
        } else {
            o->pos.vx += 8;
        }
    }
    if (o->pos.vz != z) {
        if (z < -2524) {
            o->pos.vz -= 8;
        } else {
            o->pos.vz += 8;
        }
    }
    if (o->pos.vy != y) {
        if (y >= 0) {
            o->pos.vy += 8;
        } else {
            o->pos.vy -= 8;
        }
    }
    if (o->pos.vx == x && o->pos.vz == z && o->pos.vy == y) {
        o->kill = 1;
    }
}
