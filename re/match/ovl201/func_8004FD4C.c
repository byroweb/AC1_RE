#include "ac1/ac1.h"
/* ovl201 0x8004FD4C - slide pos.vx/vy down by 4 towards (12, 20); once there,
 * an object built by func_8004F810 retires, any other moves on to
 * func_8004FDEC. */

extern void func_8004F810(void);
extern void func_8004FDEC(UiObj *o);

void func_8004FD4C(UiObj *o)
{
    if (o->pos.vx == 12 && o->pos.vy == 20) {
        if (o->ctor == func_8004F810) {
            o->kill = 1;
        } else {
            o->update = func_8004FDEC;
        }
    }
    if (o->pos.vx > 12) {
        o->pos.vx -= 4;
        if (o->pos.vx < 12) {
            o->pos.vx = 12;
        }
    }
    if (o->pos.vy > 20) {
        o->pos.vy -= 4;
        if (o->pos.vy < 20) {
            o->pos.vy = 20;
        }
    }
}
