/* ovl202 0x80085AFC: matched (no extra flags), no asm. The arms compute q = out->v - p->v and dd = d->v and share
 * `common: r = q / dd;`. Abs temps: `vx = d->vx; ax2 = vx; if (vx < 0) ax2 = -vx;` plus a dead `vx = (s16)vx;` after the
 * last use of ax2 (vx then outlives ax2, so cse keeps vx canonical: separate load reg + copy, as retail); same for vz.
 * The z-arm loads out->vz / p->z in identical arms of a free `(s32)d < 0` test: the join label ends cse2's block, so
 * d->vz is reloaded for the divisor (retail lh a1,4(s1)); jump2 merges the arms and deletes the branch. */
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
    s32 axis;
    s32 t;
    s32 big;
    s32 r;
    s32 v;
    s32 o, q, dd;
    s32 zz;
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
        s32 ax2;
        s32 vx = d->vx;
        ax2 = vx;
        if (vx < 0) {
            ax2 = -vx;
        }
        big = ax2;
        vx = (s16)vx;
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
        s32 az2;
        s32 lt;
        s32 vz = d->vz;
        az2 = vz;
        if (vz < 0) {
            az2 = -vz;
        }
        lt = big < az2;
        vz = (s16)vz;
        if (lt) {
            if ((s32)d < 0) {
                o = out->vz;
                zz = p->z;
            } else {
                o = out->vz;
                zz = p->z;
            }
            dd = d->vz;
            q = o - zz;
common:
            r = q / dd;
            p->n = r;
            if ((s16)r < 0) {
                p->n = 0;
            }
            goto done;
        }
    }
    if (axis != 0) {
        q = out->vy - p->y;
        dd = d->vy;
    } else {
        q = out->vx - p->x;
        dd = d->vx;
    }
    goto common;
done:
    return hit->ws;
miss:
    out->pad = 0;
    p->n = n;
    return 0;
}
