#include "ac1/ac1.h"

/* Relocatable block: s32 flags (bit 0 = done), s32 count, then count
 * 0x1C-byte entries whose words +0x00, +0x08, +0x10 are offsets from the
 * first entry; turned into pointers once. */

typedef struct RelocEnt {
    s32 a;
    s32 pad04;
    s32 b;
    s32 pad0c;
    s32 c;
    s32 pad14[2];
} RelocEnt;

void func_800B597C(s32 *p)
{
    s32 i;
    s32 n;
    RelocEnt *e;

    if (!(*p & 1)) {
        *p++ |= 1;
        n = *p++;
        e = (RelocEnt *)p;
        for (i = 0; i < n; i++) {
            e[i].a += (s32)e;
            e[i].b += (s32)e;
            e[i].c += (s32)e;
        }
    }
}
