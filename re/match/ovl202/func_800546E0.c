#include "ac1/ac1.h"

s32 func_800546E0(s32 base, s32 a, s32 b, s32 d) {
    s32 t;
    s32 da;
    s32 db;

    t = d + base;
    da = (t - a) & 0xFFF;
    if (((b - a) & 0xFFF) < da) {
        if (da > 0x800) {
            da = 0x1000 - da;
        }
        db = (t - b) & 0xFFF;
        if (db > 0x800) {
            db = 0x1000 - db;
        }
        if (da < db) {
            return a - base;
        }
        return b - base;
    }
    return da + a - base;
}
