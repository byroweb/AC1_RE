#include "ac1/ac1.h"
/* ovl202 0x8006E0C0 - candidate world structs along a segment: 3D DDA through the
 * 1024-unit buckets, OR-ing each cell and its forward neighbours (func_8006DFCC) */
extern s32 D_801E8B70;
void func_8006DFCC(u32 *bits, s32 x, s32 y, s32 z);



void func_8006E0C0(s32 *seg, u32 *bits)
{
    u32 *b;
    s32 i;
    s32 x;
    s32 y;
    s32 z;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 n;
    s32 sx;
    s32 sy;
    s32 sz;
    s32 m;
    s32 ex;
    s32 ey;
    s32 ez;
    s32 t;
    s32 d;

    i = D_801E8B70;
    b = bits;
    bits[16] = i;
    do {
        *b++ = 0;
    } while (--i != 0);
    t = seg[0] >> 10;
    d = (seg[3] >> 10) - t;
    x = t + 32;
    if (d > 0) {
        dx = d;
        sx = 1;
    } else {
        dx = -d;
        sx = -1;
    }
    t = seg[1] >> 10;
    d = (seg[4] >> 10) - t;
    y = t + 32;
    if (d > 0) {
        dy = d;
        sy = 1;
    } else {
        dy = -d;
        sy = -1;
    }
    t = seg[2] >> 10;
    d = (seg[5] >> 10) - t;
    z = t + 32;
    if (d > 0) {
        dz = d;
        sz = 1;
    } else {
        dz = -d;
        sz = -1;
    }
    m = dx;
    if (m < dy) {
        m = dy;
    }
    if (m < dz) {
        m = dz;
    }
    ex = ey = ez = m >> 1;
    n = m;
    for (;;) {
        func_8006DFCC(bits, x, y, z);
        if (sx != 0) {
            func_8006DFCC(bits, x + sx, y, z);
        }
        if (sy != 0) {
            func_8006DFCC(bits, x, y + sy, z);
        }
        if (sz != 0) {
            func_8006DFCC(bits, x, y, z + sz);
        }
        if (n-- <= 0) {
            break;
        }
        ex -= dx;
        if (ex <= 0) {
            x += sx;
            ex += m;
        }
        ey -= dy;
        if (ey <= 0) {
            y += sy;
            ey += m;
        }
        ez -= dz;
        if (ez <= 0) {
            z += sz;
            ez += m;
        }
    }
}
