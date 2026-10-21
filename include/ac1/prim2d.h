/* include/ac1/prim2d.h - libgpu 2D line primitives used by the overlay 202
 * HUD (radar box / map lines, func_8004FF48).  Public PsyQ layouts.
 * Include after ac1/ac1.h. */
#ifndef AC1_PRIM2D_H
#define AC1_PRIM2D_H

#include "types.h"

typedef struct LINE_F2 {        /* 0x10: flat line */
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
} LINE_F2;
AC1_ASSERT_SIZE(LINE_F2, 0x10);

#define setLineF2(p) setlen(p, 3), setcode(p, 0x40)

typedef struct LINE_F3 {        /* 0x18: flat 2-segment polyline */
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    u32 pad;                    /* 0x55555555 terminator */
} LINE_F3;
AC1_ASSERT_SIZE(LINE_F3, 0x18);

#define setLineF3(p) setlen(p, 5), setcode(p, 0x48), (p)->pad = 0x55555555

#endif
