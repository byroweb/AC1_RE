#include "ac1/ac1.h"
/* ovl201 0x8007C15C - spin and slide pos.vx by vel, braking vel by 3 a frame
 * (held at 12 / -12, as in the retail code); once pos.vx crosses 0 (or vel is
 * 0) stop at 0 and move on to func_8007C22C. */

typedef struct {
    u8 pad7c[2];
    s16 vel;                    /* +0x7E */
} SlideXWork;

extern void func_8007C22C(UiObj *o);

void func_8007C15C(UiObj *o)
{
    SlideXWork *w = (SlideXWork *)&o->w;
    s16 v;

    o->rot.vy -= 32;
    o->pos.vx += w->vel;
    v = w->vel;
    if (v < 0) {
        w->vel = v + 3;
        if (w->vel >= -12) {
            w->vel = -12;
        }
        if (o->pos.vx < 0) {
            o->pos.vx = 0;
            w->vel = 0;
            o->update = func_8007C22C;
        }
    } else if (v > 0) {
        w->vel = v - 3;
        if (w->vel < 13) {
            w->vel = 12;
        }
        if (o->pos.vx > 0) {
            o->pos.vx = 0;
            w->vel = 0;
            o->update = func_8007C22C;
        }
    } else {
        o->update = func_8007C22C;
    }
}
