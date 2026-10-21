/* include/ac1/i64.h - the overlay 201 64-bit integer helpers (TU ovl201.t66,
 * 0x800B59F4..0x800B6DE8, a small soft-float double library): a 64-bit value is
 * two words passed and returned by value (struct return through the hidden a0
 * pointer).  Include after ac1/ac1.h. */
#ifndef AC1_I64_H
#define AC1_I64_H

#include "types.h"

typedef struct I64 {
    u32 lo;
    s32 hi;
} I64;

typedef struct U64 {
    u32 lo;
    u32 hi;
} U64;

/* A soft-float double seen as its two words (same TU: the double helpers
 * test and flip the bits of the high word in registers through this). */
typedef union DW {
    double d;
    struct {
        u32 lo;
        u32 hi;
    } w;
} DW;

#endif
