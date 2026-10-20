#include "ac1/ac1.h"
/* ovl202 func_80070168 - segment / collision polygon test.  p[0..2] is the
 * segment start, p[3..5] its direction.  Intersects the segment with the
 * polygon's plane (parallel: no hit), stores the point in hit, rejects it if
 * it lies outside the segment on any axis, then projects along the dominant
 * normal axis and hands the 2D point and corners (CollPts, by value) to the triangle
 * (func_8006FF58) or quad (func_8007003C) inside test. */

extern s32 func_8006FF58(CollPt pt, CollPt a, CollPt b, CollPt c);
extern s32 func_8007003C(CollPt pt, CollPt a, CollPt b, CollPt c, CollPt d);

s32 func_80070168(u8 *verts, CollPoly *poly, s32 *p, SVECTOR *hit)
{
    s32 nx, ny, nz;
    s32 den, t;
    s32 x, y, z;
    CollPt pt, a, b, c, d;
    SVECTOR *v;
    u32 dd;

    nx = poly->n[0];
    ny = poly->n[1];
    nz = poly->n[2];
    den = nx * p[3] + ny * p[4] + nz * p[5];
    if (den == 0) {
        return 0;
    }
    den >>= 12;
    if (den == 0) {
        den = 1;
    }
    t = -(nx * p[0] + ny * p[1] + nz * p[2] + poly->d) >> 12;

    x = p[3] * t / den + p[0];
    hit->vx = x;
    if (p[3] > 0) {
        dd = x - p[0];
        if (dd > (u32)p[3]) {
            return 0;
        }
    } else {
        dd = p[0] - x;
        if (dd > (u32)-p[3]) {
            return 0;
        }
    }
    y = p[4] * t / den + p[1];
    hit->vy = y;
    if (p[4] > 0) {
        dd = y - p[1];
        if (dd > (u32)p[4]) {
            return 0;
        }
    } else {
        dd = p[1] - y;
        if (dd > (u32)-p[4]) {
            return 0;
        }
    }
    z = p[5] * t / den + p[2];
    hit->vz = z;
    if (p[5] > 0) {
        dd = z - p[2];
        if (dd > (u32)p[5]) {
            return 0;
        }
    } else {
        dd = p[2] - z;
        if (dd > (u32)-p[5]) {
            return 0;
        }
    }

    switch (poly->axis) {
    case 0:
        pt.x = z;
        pt.y = y;
        v = (SVECTOR *)(verts + (poly->v01.w & 0x7FF8));
        a.x = v->vz;
        a.y = v->vy;
        v = (SVECTOR *)(verts + ((poly->v01.w >> 16) & 0x7FF8));
        b.x = v->vz;
        b.y = v->vy;
        v = (SVECTOR *)(verts + (poly->v23 & 0x7FF8));
        c.x = v->vz;
        c.y = v->vy;
        if ((poly->v23 & 0x7FF80000) == 0x7FF80000) {
            return func_8006FF58(pt, a, b, c);
        }
        v = (SVECTOR *)(verts + ((poly->v23 >> 16) & 0x7FF8));
        d.x = v->vz;
        d.y = v->vy;
        return func_8007003C(pt, a, b, c, d);
    case 1:
        pt.x = z;
        pt.y = x;
        v = (SVECTOR *)(verts + (poly->v01.w & 0x7FF8));
        a.x = v->vz;
        a.y = v->vx;
        v = (SVECTOR *)(verts + ((poly->v01.w >> 16) & 0x7FF8));
        b.x = v->vz;
        b.y = v->vx;
        v = (SVECTOR *)(verts + (poly->v23 & 0x7FF8));
        c.x = v->vz;
        c.y = v->vx;
        if ((poly->v23 & 0x7FF80000) == 0x7FF80000) {
            return func_8006FF58(pt, a, b, c);
        }
        v = (SVECTOR *)(verts + ((poly->v23 >> 16) & 0x7FF8));
        d.x = v->vz;
        d.y = v->vx;
        return func_8007003C(pt, a, b, c, d);
    case 2:
        pt.x = y;
        pt.y = x;
        v = (SVECTOR *)(verts + (poly->v01.w & 0x7FF8));
        a.x = v->vy;
        a.y = v->vx;
        v = (SVECTOR *)(verts + ((poly->v01.w >> 16) & 0x7FF8));
        b.x = v->vy;
        b.y = v->vx;
        v = (SVECTOR *)(verts + (poly->v23 & 0x7FF8));
        c.x = v->vy;
        c.y = v->vx;
        if ((poly->v23 & 0x7FF80000) == 0x7FF80000) {
            return func_8006FF58(pt, a, b, c);
        }
        v = (SVECTOR *)(verts + ((poly->v23 >> 16) & 0x7FF8));
        d.x = v->vy;
        d.y = v->vx;
        return func_8007003C(pt, a, b, c, d);
    }
    return 0;
}
