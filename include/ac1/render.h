/* include/ac1/render.h - GPU primitives, the scratchpad render block and the
 * triangle subdivider's work stack (overlays 202/203/204).
 *
 * POLY_GT3 / P_TAG are the public PsyQ libgpu layouts.  RenderScratch is the
 * block at the start of the 1 KB scratchpad (0x1F800000) that the mesh
 * emitters read; only the fields the matched code touches are named.
 * SubVtx / SubWork are the recursive subdivider's frames (func_8006AAC8 and
 * its five clones): each level keeps its three edge midpoints and the three
 * corner pointers, and the next level's frame follows at +0x58.
 */
#ifndef AC1_RENDER_H
#define AC1_RENDER_H

#include "types.h"

typedef struct P_TAG {          /* libgpu primitive header */
    unsigned addr : 24;
    unsigned len : 8;
    u8 r0, g0, b0, code;
} P_TAG;

typedef struct POLY_GT3 {       /* 0x28: gouraud textured triangle */
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
} POLY_GT3;
AC1_ASSERT_SIZE(POLY_GT3, 0x28);

#define setlen(p, _len)   (((P_TAG *)(p))->len = (u8)(_len))
#define setcode(p, _code) (((P_TAG *)(p))->code = (u8)(_code))
#define getaddr(p)        (u32)(((P_TAG *)(p))->addr)
#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))
#define addPrim(ot, p)    setaddr(p, getaddr(ot)), setaddr(ot, p)

typedef struct RenderScratch {  /* 0x1F800000, scratchpad */
    u8 unk00[4];
    u8 *prim;                   /* +0x004 next free primitive (bumped by the size emitted) */
    u8 unk08[0x27];
    u8 code;                    /* +0x02F GPU code byte for the textured polys */
    u8 unk30[0x360 - 0x30];
    u32 clut;                   /* +0x360 clut << 16, or'ed into the u0v0 word */
    u32 tpage;                  /* +0x364 tpage << 16, or'ed into the u1v1 word */
    u8 unk368[8];
    u32 *ot;                    /* +0x370 ordering table */
    u8 unk374[4];
    s32 otshift;                /* +0x378 z >> otshift = ordering-table slot */
    u8 unk37C[0x400 - 0x37C];
} RenderScratch;
AC1_ASSERT_SIZE(RenderScratch, 0x400);

#define RSCR ((RenderScratch *)0x1F800000)

typedef struct SubVtx {         /* 0x18: one subdivider vertex */
    SVECTOR pos;                /* +0x00 model position */
    u8 u, v;                    /* +0x08 texture coordinate (read as one u16 when emitted) */
    u16 pad0A;
    u8 r, g, b, cd;             /* +0x0C colour (copied as one word when emitted) */
    s32 sxy;                    /* +0x10 screen xy (GTE SXY2) */
    u32 sz;                     /* +0x14 ordering z (SZ3 >> 2); 0 = not visible */
} SubVtx;
AC1_ASSERT_SIZE(SubVtx, 0x18);

typedef struct SubWork {        /* 0x58: one recursion level */
    SubVtx mid[3];              /* +0x00 midpoints of v0-v1, v1-v2, v2-v0 */
    SubVtx *v[3];               /* +0x48 the triangle's corners */
    s32 pad54;
} SubWork;
AC1_ASSERT_SIZE(SubWork, 0x58);

#endif
