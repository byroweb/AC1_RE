#include "ac1/ac1.h"

typedef struct {
    s32 x0, y0, z0;
    s32 x1, y1, z1;
} Seg32;

typedef struct {
    void *obj;
    s32 hit;
} HitRec;

extern void func_80014338(SVECTOR *in, void *m, SVECTOR *out);
extern s32 func_80014988(s32 x, s32 y, s32 z);
extern s32 func_8006D534(void *owner, SVECTOR *from, SVECTOR *vel, void *out);
extern HitRec *func_800705D8(Seg32 *seg, void *out, s32 n);
extern SVECTOR D_80039C78;

/* one flight step: accelerate, clamp the speed to maxspd, move, then test
 * the swept segment against the world and objects */
s32 func_80083FA0(Projectile *p, void *m, s32 maxspd, SVECTOR *vel, SVECTOR *acc, void *out)
{
    SVECTOR prev;
    SVECTOR a;
    SVECTOR unused;     /* retail frame slot, never touched */
    Seg32 seg;
    HitRec *h;
    s32 d;
    s32 r;

    if (m == 0) {
        a = D_80039C78;
    } else {
        func_80014338(acc, m, &a);
    }
    *acc = a;
    vel->vx += a.vx;
    vel->vy += a.vy;
    vel->vz += a.vz;
    d = func_80014988(vel->vx, vel->vy, vel->vz);
    if (maxspd < d) {
        vel->vx = vel->vx * maxspd / d;
        vel->vy = vel->vy * maxspd / d;
        vel->vz = vel->vz * maxspd / d;
    }
    prev = *(SVECTOR *)p;
    p->x += vel->vx;
    p->y += vel->vy;
    p->z += vel->vz;
    seg.x0 = prev.vx;
    seg.y0 = prev.vy;
    seg.z0 = prev.vz;
    seg.x1 = p->x;
    seg.y1 = p->y;
    seg.z1 = p->z;
    h = func_800705D8(&seg, out, 6);
    if (h != 0) {
        return h->hit;
    }
    r = func_8006D534(p->owner, &prev, vel, out);
    if (r != 0) {
        return r;
    }
    p->section = world_section_find(p->section, p);
    return 0;
}
