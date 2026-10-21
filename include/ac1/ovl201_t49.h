/* include/ac1/ovl201_t49.h - overlay 201 state block at 0x8017F6D0 (func_8009646C clears it,
 * func_800963A0 loads its saved half).  Include after "ac1/ac1.h". */
#ifndef AC1_OVL201_T49_H
#define AC1_OVL201_T49_H

typedef struct {
    s8 a;
    s8 b;
} S8Pair;

typedef struct {                /* 0x8017F6D0 */
    s8 a6d0[64];                /* +0x000 */
    S8Pair cur[64];             /* +0x040 0x8017F710 */
    s8 f790;                    /* +0x0C0 */
    s8 f791;
    u8 pad792[2];
    s8 f794;                    /* +0x0C4 */
    s8 f795;
    s8 f796;
    u8 pad797[9];
    s8 a7a0[64];                /* +0x0D0 0x8017F7A0 */
    S8Pair saved[64];           /* +0x110 0x8017F7E0, start of the loaded block */
} T49State;

extern T49State D_8017F6D0;

#endif
