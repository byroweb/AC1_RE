/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins): Manhattan distance from
 * the entity position to p. */
#include "ac1/ac1.h"
int abs(int);
s32 func_801C8C68(AcEntity *e, AcPos *p)
{
    s32 dx = p->x - e->pos.x, dy = p->y - e->pos.y, dz = p->z - e->pos.z;
    return abs(dx) + abs(dy) + abs(dz);
}
