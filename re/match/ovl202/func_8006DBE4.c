#include "ac1_types.h"
/* clear bit x in rows y0..y1 of a bitmap with 64-byte (16-word) rows */
void func_8006DBE4(u32 (*map)[16], s32 y0, s32 y1, u32 x)
{
    u32 *p;
    s32 n;
    u32 bit;
    n = y1 - y0;
    p = &map[y0][x >> 5];
    bit = ~(1 << (x & 0x1F));
    while (n-- >= 0) {
        *p &= bit;
        p += 16;
    }
}
