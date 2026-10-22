#include "ac1/ac1.h"

s16 func_800132CC(s32 cur, s32 tgt, s32 step)
{
    s16 a;
    s16 b;
    s16 r;

    a = cur & 0xFFF;
    b = tgt & 0xFFF;
    r = a;
    if (b == a) {
        return b;
    }
    if (a < b) {
        if (b - a <= 0x800) {
            r = a + step;
            if (b < r) {
                return b;
            }
        } else {
            r = (a - step) & 0xFFF;
            if (r >= 0x800 && b >= r) {
                return b;
            }
        }
    } else {
        if (a - b <= 0x800) {
            r = a - step;
            if (r < b) {
                return b;
            }
        } else {
            r = (a + step) & 0xFFF;
            if (r <= 0x800 && r >= b) {
                return b;
            }
        }
    }
    return r;
}
