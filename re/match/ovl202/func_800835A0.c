#include "ac1/ac1.h"

extern u16 D_8008EF4A;
extern u8 D_8019F518;
extern u16 D_800906CC[];

void func_800835A0(void) {
    Projectile *p;
    s32 i;

    p = g_proj_pool;
    i = 0x100;
    D_8008EF4A = D_800906CC[D_8019F518];
    do {
        p->free = -1;
        i -= 1;
        p++;
    } while (i != 0);
}
