#include "ac1/ac1.h"
/* ovl201 0x8004F4A4 - step pos 8 units a frame towards (0, -64, -2524) while
 * spinning; at rest set state 1 and park on ui_obj_nop2.  (The second rest
 * test reads vy == 0, as in the retail code.) */

void func_8004F4A4(UiObj *o)
{
    if (o->pos.vx == 0 && o->pos.vy == -64 && o->pos.vz == -2524) {
        o->w.seq.state = 1;
        o->update = ui_obj_nop2;
        return;
    }
    if (o->pos.vx != 0) {
        if (o->pos.vx > 0) {
            o->pos.vx -= 8;
        } else {
            o->pos.vx += 8;
        }
    }
    if (o->pos.vz != -2524) {
        if (o->pos.vz > -2524) {
            o->pos.vz -= 8;
        } else {
            o->pos.vz += 8;
        }
    }
    if (o->pos.vy != -64) {
        if (o->pos.vy > -64) {
            o->pos.vy -= 8;
        } else {
            o->pos.vy += 8;
        }
    }
    o->rot.vy -= 64;
    if (o->pos.vx == 0 && o->pos.vy == 0 && o->pos.vz == -2524) {
        o->w.seq.state = 1;
        o->update = ui_obj_nop2;
    }
}
