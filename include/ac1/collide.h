/* include/ac1/collide.h - overlay 202 collision probe: mesh table, polygon
 * records and the scratchpad work block of the nearest-hit search.
 *
 * Evidence: func_800705D8 (the search, two clones in 203/204) and the shapes
 * of its callees func_80070168 (one polygon), func_80070A58 (bucket mask),
 * func_8006FB6C (bitset iterator), func_8006FA30 (hit point fix-up).
 */
#ifndef AC1_COLLIDE_H
#define AC1_COLLIDE_H

#include "types.h"
#include "mission.h"

typedef struct CollPoly {       /* 0x14: one collision polygon */
    u8 unk00[0x0E];
    u16 flags;                  /* +0x0E low 3 bits: collision classes (masked by the caller) */
    u8 unk10[4];
} CollPoly;
AC1_ASSERT_SIZE(CollPoly, 0x14);

typedef struct CollHdr {        /* polygon set header; the CollPoly array starts at +0x64 */
    u16 n;                      /* +0x00 polygon count; 16+ = bucketed in groups of (n >> 5) + 1 */
    u8 unk02[0x62];
} CollHdr;
AC1_ASSERT_SIZE(CollHdr, 0x64);

typedef struct CollMesh {       /* 8: entry of the mesh table at 0x80091228 */
    void *verts;                /* +0x00 vertex data handed to the polygon test */
    CollHdr *hdr;               /* +0x04 */
} CollMesh;
AC1_ASSERT_SIZE(CollMesh, 8);

typedef struct CollHit {        /* 0x801E8B74: result of the last successful probe */
    CollPoly *poly;
    WorldStruct *ws;
} CollHit;
AC1_ASSERT_SIZE(CollHit, 8);

typedef struct CollScratch {    /* 0x1F800000, scratchpad, during a probe */
    u32 bits[17];               /* +0x00 candidate world structs (bitset, func_8006E0C0) */
    s32 unk44[4];               /* +0x44 handed to func_80070A58 */
    s32 p[3];                   /* +0x54 probe start */
    s32 d[3];                   /* +0x60 probe end, then end - start */
    SVECTOR hit;                /* +0x6C hit point of the last polygon test */
    SVECTOR best_hit;           /* +0x74 nearest hit point so far */
    CollMesh *meshes;           /* +0x7C the mesh table */
    s32 idx;                    /* +0x80 current world struct */
    s32 group;                  /* +0x84 polygons per bucket bit */
    s32 best_dist;              /* +0x88 Manhattan distance of best_hit */
    WorldStruct *ws;            /* +0x8C */
    WorldStruct *best_ws;       /* +0x90 */
    CollMesh *mesh;             /* +0x94 */
    CollMesh *best_mesh;        /* +0x98 */
    CollHdr *hdr;               /* +0x9C */
    CollPoly *cur;              /* +0xA0 */
    CollPoly *end;              /* +0xA4 end of the polygon array */
    CollPoly *group_end;        /* +0xA8 end of the current bucket */
    CollPoly *best_poly;        /* +0xAC 0 = nothing hit */
    u32 mask;                   /* +0xB0 buckets still to test, lowest bit first */
} CollScratch;
AC1_ASSERT_SIZE(CollScratch, 0xB4);

#define CSCR ((CollScratch *)0x1F800000)

#endif
