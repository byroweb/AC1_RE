#include "ac1/ac1.h"
/* ovl202 func_80070A58 - which polygon buckets of a mesh a segment touches.
 * The segment p[0..2] -> p[3..5] is mapped into the bounding box's 8 x 8 x 8
 * cell grid and walked with a 3D Bresenham along its major axis (clipped to
 * the grid); at every step the cell and its three forward neighbours are
 * looked up with func_80070A00 and their bucket bits are or'ed together. */

extern u32 func_80070A00(CollHdr *hdr, s32 x, s32 y, s32 z);

u32 func_80070A58(CollHdr *hdr, s32 *p, CollBox *box)
{
    s32 x0, y0, z0;
    s32 x1, y1, z1;
    s32 dx, dy, dz;
    s32 adx, ady, adz;
    s32 ex, ey, ez;
    s32 sx, sy, sz;
    s32 w, n, axis, k, cnt;
    u32 mask;

    x0 = p[0] - box->min.vx;
    x1 = p[3] - box->min.vx;
    w = box->max.vx - box->min.vx;
    if (w == 0) {
        x0 = x1 = 0;
    } else {
        x0 = (x0 << 3) / w;
        x1 = (x1 << 3) / w;
    }
    y0 = p[1] - box->min.vy;
    y1 = p[4] - box->min.vy;
    w = box->max.vy - box->min.vy;
    if (w == 0) {
        y0 = y1 = 0;
    } else {
        y0 = (y0 << 3) / w;
        y1 = (y1 << 3) / w;
    }
    z0 = p[2] - box->min.vz;
    z1 = p[5] - box->min.vz;
    w = box->max.vz - box->min.vz;
    if (w == 0) {
        z0 = z1 = 0;
    } else {
        z0 = (z0 << 3) / w;
        z1 = (z1 << 3) / w;
    }

    dx = x1 - x0;
    sx = (dx > 0) ? (adx = dx, 1) : (adx = -dx, -1);
    dy = y1 - y0;
    sy = (dy > 0) ? (ady = dy, 1) : (ady = -dy, -1);
    dz = z1 - z0;
    sz = (dz > 0) ? (adz = dz, 1) : (adz = -dz, -1);

    n = adx;
    axis = 0;
    if (n < ady) {
        n = ady;
        axis = 1;
    }
    if (n < adz) {
        n = adz;
        axis = 2;
    }
    ex = ey = ez = n >> 1;
    cnt = n;
    if (n > 0) {
        switch (axis) {
        case 0:
            if (x0 < 0) {
                k = -x0;
                x0 = 0;
            clipx:
                y0 += dy * k / n;
                z0 += dz * k / n;
                cnt -= k;
            } else if (x0 >= 8) {
                k = x0 - 7;
                x0 = 7;
                goto clipx;
            }
            if (x1 < 0) {
                cnt += x1;
            } else if (x1 >= 8) {
                cnt += 7 - x1;
            }
            break;
        case 1:
            if (y0 < 0) {
                k = -y0;
                y0 = 0;
            clipy:
                x0 += dx * k / n;
                z0 += dz * k / n;
                cnt -= k;
            } else if (y0 >= 8) {
                k = y0 - 7;
                y0 = 7;
                goto clipy;
            }
            if (y1 < 0) {
                cnt += y1;
            } else if (y1 >= 8) {
                cnt += 7 - y1;
            }
            break;
        case 2:
            if (z0 < 0) {
                k = -z0;
                z0 = 0;
            clipz:
                x0 += dx * k / n;
                y0 += dy * k / n;
                cnt -= k;
            } else if (z0 >= 8) {
                k = z0 - 7;
                z0 = 7;
                goto clipz;
            }
            if (z1 < 0) {
                cnt += z1;
            } else if (z1 >= 8) {
                cnt += 7 - z1;
            }
            break;
        }
    }

    mask = 0;
    for (;;) {
        mask |= func_80070A00(hdr, x0, y0, z0);
        mask |= func_80070A00(hdr, x0 + sx, y0, z0);
        mask |= func_80070A00(hdr, x0, y0 + sy, z0);
        mask |= func_80070A00(hdr, x0, y0, z0 + sz);
        if (cnt-- <= 0) {
            break;
        }
        ex -= adx;
        if (ex <= 0) {
            x0 += sx;
            ex += n;
        }
        ey -= ady;
        if (ey <= 0) {
            y0 += sy;
            ey += n;
        }
        ez -= adz;
        if (ez <= 0) {
            z0 += sz;
            ez += n;
        }
    }
    return mask;
}
