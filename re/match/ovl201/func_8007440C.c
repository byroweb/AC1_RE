#include "ac1/ac1.h"
/* ovl201 0x8007440C - count down the shared timer byte; at zero clear the
 * flag byte and retire, else scroll the three D_800B9088 coordinates by 128
 * (64 while D_801A2574 is 0). */

typedef struct {
    u8 *flag;                   /* +0x7C */
    u8 *timer;                  /* +0x80 */
} ScrollWork;

extern u16 D_800B9088[];
extern s32 D_801A2574;

void func_8007440C(UiObj *o)
{
    ScrollWork *w = (ScrollWork *)&o->w;
    s32 d;

    if (--*w->timer == 0) {
        *w->flag = 0;
        o->kill = 1;
        return;
    }
    d = 128;
    if (D_801A2574 == 0) {
        d = 64;
    }
    D_800B9088[0] += d;
    D_800B9088[3] += d;
    D_800B9088[6] += d;
}
