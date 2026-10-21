/* ovl202 func_80085764 = proj_init */
#include "ac1/ac1.h"

void func_80085764(Projectile *p, void *owner, AcPos *pos, s32 section)
{
    p->owner = owner;
    *(AcPos *)p = *pos;
    p->free = 0;
    p->unk0c = -1;
    p->unk11 = 0xFF;
    p->kind = 0;
    p->unk12 = -1;
    p->think = 0;
    if (section == -1) {
        p->section = world_section_find(-1, p);
    } else {
        p->section = section;
    }
}
