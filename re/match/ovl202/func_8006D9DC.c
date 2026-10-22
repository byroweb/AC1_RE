#include "ac1/ac1.h"

typedef struct {
    s32 a;
    s32 b;
} RelocPair;

extern RelocPair D_80091228[];

void func_8006D9DC(void)
{
    s32 i;
    RelocPair *p;

    i = 0x400;
    p = D_80091228;
    do {
        i--;
        p->a += (s32)D_80091228;
        p->b += (s32)D_80091228;
        p++;
    } while (i != 0);
}
