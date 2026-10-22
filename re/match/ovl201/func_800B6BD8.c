#include "ac1/ac1.h"
#include "ac1/i64.h"

extern I64 func_800B5D74(I64 a, I64 b);

/* Full 32 x 32 -> 64 unsigned product from 16-bit partial products. */
I64 func_800B6BD8(u32 a, u32 b) {
    I64 r;
    I64 t;
    u32 al;
    u32 bl;
    u32 p;

    al = a & 0xFFFF;
    bl = b & 0xFFFF;
    r.lo = al * bl;
    r.hi = 0;
    a >>= 16;
    p = a * bl;
    t.hi = p >> 16;
    t.lo = p << 16;
    r = func_800B5D74(r, t);
    b >>= 16;
    p = al * b;
    t.hi = p >> 16;
    t.lo = p << 16;
    r = func_800B5D74(r, t);
    r.hi += a * b;
    return r;
}
