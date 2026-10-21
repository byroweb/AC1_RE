/* include/ac1/ovl202_t42.h - overlay 202 map/HUD marker code (TU t42): the scratchpad
 * layout at 0x1F800000 and projected points.  Include after "ac1/ac1.h". */
#ifndef AC1_OVL202_T42_H
#define AC1_OVL202_T42_H

typedef struct {                /* 8: projected point */
    s16 x;
    s16 y;
    s16 z;                      /* OTZ (the GTE stores a word here) */
    s16 clip;                   /* 255 = behind; bit0 left, 1 right, 2 above, 3 below */
} ScrPt;

typedef struct {                /* scratchpad 0x1F800000 (ovl202 TU t42) */
    MATRIX m;                   /* +0x000 */
    u8 pad020[0x100];
    SVECTOR org;                /* +0x120 */
    SVECTOR v;                  /* +0x128 */
    VECTOR out;                 /* +0x130 */
    u32 *ot;                    /* +0x140 */
    u8 *cur;                    /* +0x144 next free primitive */
    MATRIX rot;                 /* +0x148 */
    s32 flag;                   /* +0x168 */
    u8 pad16c[4];
    ScrPt pts[3];               /* +0x170 */
} T42Spad;

#define SPAD ((T42Spad *)0x1F800000)

#endif
