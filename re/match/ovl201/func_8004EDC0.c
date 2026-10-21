#include "ac1/ac1.h"
/* ovl201 0x8004EDC0 - slide the bar object left (6 or 10 a frame) until it
 * reaches its stop x, then move on to func_8004EE7C. */

extern s16 func_800517C8(void);
extern void func_8004EE7C(UiObj *o);

void func_8004EDC0(UiObj *o)
{
    UiWork *w = &o->w;

    if (w->slide.mode == 0) {
        if (func_800517C8()) {
            w->slide.bar->pos.vx -= 6;
        } else {
            w->slide.bar->pos.vx -= 6;
        }
    } else {
        w->slide.bar->pos.vx -= 10;
    }
    if (w->slide.bar->pos.vx <= w->slide.stop) {
        w->slide.bar->pos.vx = w->slide.stop;
        o->update = func_8004EE7C;
    }
}
