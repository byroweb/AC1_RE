#include "ac1/ac1.h"
typedef struct {
    u16 x, y, z;
    u8 pad[0x12];
    u16 vx, vy, vz;
    u16 padv;
    u16 ax, ay, az;
    u8 pad2[0x96];
    u16 cnt;
} P;
void func_801C6C5C(P *a)
{
a->x += a->vx;
a->y += a->vy;
a->z += a->vz;
a->cnt++;
a->vx += a->ax;
a->vy += a->ay;
a->vz += a->az;
}
