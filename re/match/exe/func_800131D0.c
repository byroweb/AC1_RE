#include "ac1/ac1.h"

typedef struct {
    s16 q;
    s16 q1;
    s16 half;
    s16 r;
} DivRes;

void func_800131D0(DivRes *p, s32 d, s32 n)
{
    s32 t;

    p->half = (d + 1) >> 1;
    p->q = n / d;
    p->r = __builtin_abs(n % d);
    t = p->q;
    if (n >= 0) {
        p->q1 = t + 1;
    } else {
        p->q1 = t - 1;
    }
}
