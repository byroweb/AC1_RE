#include "ac1/ac1.h"

extern s32 rand(void);

void func_8008938C(Projectile *p)
{
    s16 fade;   /* unused; its slot is the retail 8-byte frame pad */

    if (--*(s16 *)&p->unk2a[0] == 0) {
        p->unk12 = 0;
    }
    p->x += p->spin[0];
    p->y += p->spin[1] - ((rand() * 48) >> 15);
    p->z += p->spin[2];
    p->section = world_section_find(p->section, p);
    p->ang[2] += *(s16 *)&p->dtint[2];
    p->unkbe += p->vel[0];
    if (p->vel[1] != 0) {
        ((u8 *)p->ang)[0] -= p->vel[1];
        ((u8 *)p->ang)[1] -= p->vel[1];
        ((u8 *)p->ang)[2] -= p->vel[1];
    }
}
