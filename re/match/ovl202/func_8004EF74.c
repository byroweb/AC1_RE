#include "ac1/ac1.h"
/* ovl202 0x8004EF74 - count a timer down by its rate; on expiry reload it by
 * period and return base + alt, else base + cur. */

typedef struct {
    s16 cur;
    s16 alt;
    s16 timer;
    s16 rate;
} Tick;

s32 func_8004EF74(Tick *t, s32 base, s32 period)
{
    s16 tm;
    s32 r;

    tm = t->timer - t->rate;
    t->timer = tm;
    if (tm <= 0) {
        t->timer = tm + period;
        r = base + t->alt;
    } else {
        r = base + t->cur;
    }
    return r;
}
