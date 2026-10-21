#include "ac1/ac1.h"
/* ovl202 0x8006DE54 - OR together the y-bucket rows lo..hi (1024 units each) of a
 * bucket table into out; row width and gap come from the scratchpad */
typedef struct BitScr {
    u32 unk00[16];
    s32 n;                      /* +0x40 words per row */
    s32 skip;                   /* +0x44 words from one row's end to the next row */
} BitScr;


void func_8006DE54(u32 *tab, s16 lo, s16 hi, u32 *out)
{
    BitScr *s;
    u32 *p;
    u32 *o;
    s32 n;
    s32 k;

    s = (BitScr *)0x1F800000;
    p = (u32 *)((u8 *)tab + 0x800 + ((lo >> 10) << 6));
    o = out;
    n = s->n;
    do {
        *o++ = *p++;
    } while (--n != 0);
    k = (hi >> 10) - (lo >> 10);
    while (p += s->skip, k-- > 0) {
        o = out;
        n = s->n;
        do {
            *o++ |= *p++;
        } while (--n != 0);
    }
}
