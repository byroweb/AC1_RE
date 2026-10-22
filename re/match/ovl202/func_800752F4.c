#include "ac1/ac1.h"

typedef struct {
    u8 kind;
    u8 pad;
    u16 val;
} ScaleItem;

s32 func_800752F4(u32 flags, ScaleItem *it)
{
    s32 t;
    s32 n;
    s32 s;

    t = it->kind & 0x30;
    n = it->val;
    if (t == 0x20) {
        if (n == 0) {
            return 0;
        }
        s = (flags & 0xF00) >> 8;
        if (s >= 8) {
            s -= 16;
        }
        n = ((n * (8 - s)) >> 3) + 1;
    }
    return n;
}
