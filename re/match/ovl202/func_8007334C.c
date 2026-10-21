#include "ac1/ac1.h"

typedef struct {
    u16 h[4];
    u16 e[8];
} Rec24;

extern Rec24 D_80198808[];

void func_8007334C(s32 *arg0) {
    Rec24 *d;
    s32 n;
    u16 *src;
    s32 i;
    u16 *p;

    d = D_80198808;
    n = *arg0;
    src = (u16 *)(arg0 + 1);
    while (--n != -1) {
        d->h[0] = *src++;
        d->h[1] = *src++;
        d->h[2] = *src++;
        d->h[3] = *src++;
        i = 8;
        p = d->e;
        for (; i != 0; i--) {
            *p++ = *src++;
        }
        d++;
    }
}
