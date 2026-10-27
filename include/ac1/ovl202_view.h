/* include/ac1/ovl202_view.h - scratchpad (0x1F800000) layout of the overlay 202
 * world-object view: func_8008D3B0 fills the matrices around the player and calls
 * the drawers 0x8008CA14 / 0x8008D288 / 0x8008CB5C / 0x8008CC88 (func_8008CC88 is the
 * flat-lit F3/F4 mesh renderer).  Opt-in: include after ac1/ac1.h.
 *
 * The scratchpad is not one structure: 81 matched sources declare about 50 distinct
 * layouts for it, each private to one routine and its overlay copies (render.h's
 * RenderScratch, collide.h's CollScratch and ovl202_t42.h's T42Spad are three
 * more).  This header is the one view two separate routines share. */
#ifndef AC1_OVL202_VIEW_H
#define AC1_OVL202_VIEW_H

#include "types.h"

typedef struct ViewVtx {        /* 8: one transformed vertex */
    s16 x, y;                   /* +0x00 SXY2 */
    s16 z;                      /* +0x04 SZ3 >> 2 (stored as a word) */
    u16 clip;                   /* +0x06 outcode: 1/2 left/right, 4/8 top/bottom, 0xFF behind */
} ViewVtx;
AC1_ASSERT_SIZE(ViewVtx, 8);

typedef struct ViewModel {      /* 0x1C: TMD object-table entry (model table at *0x801A5F2C + 0xC) */
    SVECTOR *verts;             /* +0x00 vert_top */
    u16 nverts;                 /* +0x04 */
    u16 pad06;
    u8 *normals;                /* +0x08 normal_top */
    u8 pad0C[4];                /* +0x0C n_normal */
    u32 *prims;                 /* +0x10 primitive_top */
    u16 nprims;                 /* +0x14 */
    u16 pad16;
    u8 pad18[4];                /* +0x18 scale */
} ViewModel;
AC1_ASSERT_SIZE(ViewModel, 0x1C);

typedef struct ViewSpad {
    MATRIX m0;                  /* +0x000 world -> view */
    MATRIX rot[4];              /* +0x020 per-quadrant rotations, picked by -pos.pad & 3 */
    MATRIX light[4];            /* +0x0A0 light matrices: [0] from D_8009087C, [1..3] each derived
                                 *        from the previous one by func_800135E8 */
    SVECTOR cam;                /* +0x120 player position >> 2 (func_8008D3B0 calls it rot) */
    SVECTOR pos;                /* +0x128 object origin >> 2, relative to cam */
    VECTOR wpos;                /* +0x130 = t[] of the MATRIX at +0x11C */
    u32 *ot;                    /* +0x140 */
    u8 *prim;                   /* +0x144 primitive cursor */
    ViewModel *models;          /* +0x148 */
    union { u32 w; u8 b[4]; } cur; /* +0x14C current packet header word */
    u8 *normals;                /* +0x150 */
    ViewVtx vtx[1];             /* +0x154 transformed vertices, as many as the model has */
} ViewSpad;
AC1_ASSERT_SIZE(ViewSpad, 0x15C);

#endif
