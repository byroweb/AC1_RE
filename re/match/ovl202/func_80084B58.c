#include "ac1/ac1.h"

typedef struct {
    SVECTOR lo;
    SVECTOR hi;
} Box16;

extern s32 func_8006D5BC(void *owner, SVECTOR *c, s32 r, s32 h);
extern s32 func_80070F30(Box16 *box, SVECTOR *step, s32 a, s32 b, s32 c);

/* move p by `move` after sweeping a box of half-width r and height h
 * step->pad times along `step`; on a hit p stops where it hit. */
s32 func_80084B58(Projectile *p, SVECTOR *move, SVECTOR *step, s32 r, s32 h)
{
    Box16 box;
    SVECTOR c;
    s32 n;
    s32 hit;

    box.lo.vx = p->x - r;
    box.lo.vz = p->z - r;
    box.lo.vy = p->y - (h >> 1);
    box.hi.vx = p->x + r;
    box.hi.vz = p->z + r;
    box.hi.vy = p->y + (h >> 1);
    c = *(SVECTOR *)p;
    c.vy += h >> 1;
    n = step->pad;
    do {
        box.lo.vx += step->vx;
        box.hi.vx += step->vx;
        c.vx += step->vx;
        box.lo.vy += step->vy;
        box.hi.vy += step->vy;
        c.vy += step->vy;
        box.lo.vz += step->vz;
        box.hi.vz += step->vz;
        c.vz += step->vz;
        if (func_80070F30(&box, step, 0, -1, 6) != 0) {
            hit = -1;
        } else {
            hit = func_8006D5BC(p->owner, &c, r, h);
            if (hit == 0) {
                continue;
            }
        }
        *(SVECTOR *)p = c;
        p->y -= h >> 1;
        return hit;
    } while (--n != 0);
    p->x += move->vx;
    p->y += move->vy;
    p->z += move->vz;
    p->section = world_section_find(p->section, p);
    return 0;
}
