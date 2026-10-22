#include "ac1/ac1.h"
#include "ac1/i64.h"

/* a + b, carried through 16-bit halves of the low word. */
I64 func_800B5D74(I64 a, I64 b) {
    u32 x[6];

    x[1] = a.lo >> 16;
    x[0] = a.lo & 0xFFFF;
    x[3] = b.lo >> 16;
    x[2] = b.lo & 0xFFFF;
    x[0] = x[0] + x[2];
    if (x[0] & 0x10000) {
        x[1]++;
    }
    x[1] = x[1] + x[3];
    if (x[1] & 0x10000) {
        a.hi++;
    }
    a.lo = (x[1] << 16) | (x[0] & 0xFFFF);
    a.hi += b.hi;
    return a;
}
