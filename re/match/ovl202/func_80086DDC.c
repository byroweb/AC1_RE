#include "ac1/ac1.h"

extern s32 func_80014D78(s32 range);
extern void func_80086C9C(void *, AcPos *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 rand(void);

/* n debris pieces scattered around pos in X/Z; the first one gets `first` */
void func_80086DDC(void *owner, AcPos *pos, s32 spread, s32 n, s32 kind, s32 sp0, s32 up0,
                   s32 sprand, s32 uprand, s32 first)
{
    AcPos at;
    s32 pad[2];         /* retail frame slots, never touched */
    s32 i;
    s32 d;
    s32 sp;
    s32 up;

    i = n;
    i = i - 1;
    d = spread * 2;
    at.y = pos->y;
    if (n != 0) {
        do {
            at.x = pos->x + func_80014D78(d) - spread;
            at.z = pos->z + func_80014D78(d) - spread;
            sp = sp0 + func_80014D78(sprand);
            up = up0 + func_80014D78(uprand);
            i--;
            func_80086C9C(owner, &at, kind, sp, up, sp >> 3, up >> 2, -sp >> 6,
                          -(up / ((rand() & 7) + 27)), up >> 2, first, 0);
            first = -1;
        } while (i != -1);
    }
}
