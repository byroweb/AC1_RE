/* ovl202 0x80085AFC: matched (no extra flags). register asm locals pin the abs temps, big/axis, r and the arm operands; a memory-clobber asm before the dispatch stops CSE of d->vz; the div tail lives in the z-arm (goto common) and return hit->ws sits after the arms. */
#include "ac1/ac1.h"
/* ovl202 0x80085AFC: sweep a moving point (pos, velocity d) against the world collision:
 * probe pos(-back*k) .. pos + d*n (in two halves when range > 15000); on a hit store the
 * hit point in out, out->pad = 2/1 (polygon flag bit 0), pos->n = frames to impact along
 * the dominant axis; return the hit world struct (0 = miss, pos->n = n). */
typedef struct {
    s16 x, y, z;
    u8 pad06[0x12 - 0x06];
    s16 n;              /* +0x12 */
} Mover85;
typedef struct {
    s32 from[3];
    s32 to[3];
} Seg;
extern long SquareRoot0(long a);
extern CollHit *func_800705D8(Seg *seg, SVECTOR *out, s32 flags);
int abs(int);

WorldStruct *func_80085AFC(Mover85 *p, SVECTOR *d, SVECTOR *b, s32 k, s32 range, SVECTOR *out)
{
    SVECTOR unused;
    Seg seg;
    CollHit *hit;
    s32 len, n;
    s32 ax, ay, az, sx, sy, sz;
    register s32 axis asm("$5");
    s32 t;
    register s32 big asm("$4");
    register s32 r asm("$2");
    s32 v;
    s32 o, q, dd;
    s32 px, py, pz;

    len = SquareRoot0(d->vx * d->vx + d->vy * d->vy + d->vz * d->vz);
    if (len == 0) {
        p->n = 0;
        return;
    }
    n = range / len + 1;
    seg.from[0] = px = p->x;
    seg.from[1] = py = p->y;
    seg.from[2] = pz = p->z;
    if (k != 0) {
        seg.from[0] = px - ((b->vx * k) >> 12);
        seg.from[1] = py - ((b->vy * k) >> 12);
        seg.from[2] = pz - ((b->vz * k) >> 12);
    }
    if (range > 15000) {
        sx = d->vx * n;
        sy = d->vy * n;
        sz = d->vz * n;
        seg.to[0] = p->x + (sx >> 1);
        seg.to[1] = p->y + (sy >> 1);
        seg.to[2] = p->z + (sz >> 1);
        hit = func_800705D8(&seg, out, 6);
        if (hit == 0) {
            seg.from[0] = seg.to[0];
            seg.from[1] = seg.to[1];
            seg.from[2] = seg.to[2];
            seg.to[0] = p->x + sx;
            seg.to[1] = p->y + sy;
            seg.to[2] = p->z + sz;
            hit = func_800705D8(&seg, out, 6);
            if (hit != 0)
                goto found;
            goto miss;
        }
    } else {
        seg.to[0] = p->x + d->vx * n;
        seg.to[1] = p->y + d->vy * n;
        seg.to[2] = p->z + d->vz * n;
        hit = func_800705D8(&seg, out, 6);
        if (hit == 0)
            goto miss;
    }
found:
    v = hit->poly->v01.h.hi & 1;
    if (v == 0) {
        v = 1;
    } else {
        v = 2;
    }
    out->pad = v;
    {
        register s32 ax2 asm("$3");
        ax2 = d->vx;
        if (d->vx < 0) {
            ax2 = -d->vx;
        }
        big = ax2;
    }
    axis = 0;
    if (d->vy >= 0) {
        t = d->vy;
    } else {
        t = -d->vy;
    }
    if (big < t) {
        big = t;
        axis = 1;
    }
    {
        register s32 az2 asm("$3");
        register s32 lt asm("$2");
        az2 = d->vz;
        if (d->vz < 0) {
            az2 = -d->vz;
        }
        __asm__ volatile("" : : : "memory");
        lt = big < az2;
        if (lt) {
            {
                register s32 o_ asm("$3") = out->vz;
                register s32 p_ asm("$2") = p->z;
                register s32 d_ asm("$5") = d->vz;
                r = (o_ - p_) / d_;
            }
common:
            p->n = r;
            if ((s16)r < 0) {
                p->n = 0;
            }
            goto done;
        }
    }
    if (axis != 0) {
        {
                register s32 o_ asm("$3") = out->vy;
                register s32 p_ asm("$2") = p->y;
                register s32 d_ asm("$5") = d->vy;
                r = (o_ - p_) / d_;
            }
    } else {
        {
                register s32 o_ asm("$3") = out->vx;
                register s32 p_ asm("$2") = p->x;
                register s32 d_ asm("$5") = d->vx;
                r = (o_ - p_) / d_;
            }
    }
    goto common;
done:
    return hit->ws;
miss:
    out->pad = 0;
    p->n = n;
    return 0;
}
