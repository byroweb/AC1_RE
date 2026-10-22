#include "ac1/ac1.h"

extern s32 rsin(s32);
extern s32 rcos(s32);

void func_80013AFC(s16 a, MATRIX *m)
{
    s16 s;
    s16 c;

    s = rsin(a);
    c = rcos(a);
    m->m[0][2] = s;
    m->m[0][0] = c;
    m->m[0][1] = 0;
    m->m[1][0] = 0;
    m->m[1][1] = 0x1000;
    m->m[1][2] = 0;
    m->m[2][0] = -s;
    m->m[2][1] = 0;
    m->m[2][2] = c;
}
