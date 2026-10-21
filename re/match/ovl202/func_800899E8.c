#include "ac1/ac1.h"

void func_800899E8(Projectile *p)
{
    p->x += p->spin[0];
    p->y += p->spin[1];
    p->z += p->spin[2];
    p->section = world_section_find(p->section, p);
    p->ang[0] = ((0xC0 - *(s16 *)&p->unk2a[0]) & 3) + 0x11C;
    if (--*(s16 *)&p->unk2a[0] == 0) {
        p->unk12 = 0;
    }
}
