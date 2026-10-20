/* include/ac1/types.h - primitive and PsyQ libgte geometry types.
 *
 * GCC 2.7.2 safe (no // comments, no C99).  The primitive typedefs use the
 * same guard as tools/match/include/ac1_types.h, so a file may include both
 * in either order.  SVECTOR / VECTOR / DVECTOR / MATRIX are the public
 * PsyQ libgte layouts (8 / 16 / 4 / 32 bytes).
 */
#ifndef AC1_TYPES_H
#define AC1_TYPES_H
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef short s16;
typedef int s32;
#endif

#ifndef AC1_GTE_TYPES_H
#define AC1_GTE_TYPES_H

typedef struct SVECTOR {        /* 8 bytes */
    s16 vx;
    s16 vy;
    s16 vz;
    s16 pad;
} SVECTOR;

typedef struct VECTOR {         /* 16 bytes */
    s32 vx;
    s32 vy;
    s32 vz;
    s32 pad;
} VECTOR;

typedef struct DVECTOR {        /* 4 bytes: 2D point (screen) */
    s16 vx;
    s16 vy;
} DVECTOR;

typedef struct MATRIX {         /* 32 bytes: 3x3 rotation (4.12) + translation */
    s16 m[3][3];
    s32 t[3];
} MATRIX;

#endif

/* Compile-time size check.  Emits no code; m2c's C parser does not evaluate
 * sizeof, so tools/match/ctx.py defines M2C and the checks drop out. */
#ifndef AC1_ASSERT_SIZE
#ifdef M2C
#define AC1_ASSERT_SIZE(T, n)
#else
#define AC1_ASSERT_SIZE(T, n) typedef char T##_size_is_##n[(sizeof(T) == (n)) ? 1 : -1]
#endif
#endif
