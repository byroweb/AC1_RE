#include "ac1/ac1.h"

void func_80089D34(Projectile *p)
{
    p->x += p->spin[0];
    p->y += p->spin[1];
    p->z += p->spin[2];
    p->section = world_section_find(p->section, p);
    p->unkbe += 2;
    ((u8 *)p->ang)[0] -= 2;
    ((u8 *)p->ang)[1] -= 4;
    ((u8 *)p->ang)[2] -= 4;
    if (--*(s16 *)&p->unk2a[0] == 0) {
        p->unk12 = 0;
    }
}
