#include "ac1/ac1.h"
/* ovl202 0x80084A98: split a s16 vector into n equal steps (n from the larger |component|) */
int abs(int);
void func_80084A98(SVECTOR *v, int sxz, int sy, SVECTOR *out)
{
    int ax;
    int ay;
    int az;
    int a;
    int b;

    ax = abs(v->vx);
    ay = abs(v->vy);
    az = abs(v->vz);
    if (ax < az) ax = az;
    a = (ax >> 1) / sxz;
    b = ay / sy;
    if (a < b) a = b;
    a++;
    out->pad = a;
    out->vx = v->vx / a;
    out->vy = v->vy / a;
    out->vz = v->vz / a;
}
