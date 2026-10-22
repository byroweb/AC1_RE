#include "ac1/ac1.h"

extern u32 D_801EA790[3][64][16];

typedef struct {
    u32 bits[16];
    s32 n;
    s32 pad44[15];
    u32 tmp[16];
} MaskSet;

void func_8006DFCC(MaskSet *m, u32 a, u32 b, u32 c)
{
    u32 *pa;
    u32 *pb;
    u32 *pc;
    u32 *src;
    u32 *dst;
    u32 *t;
    s32 i;

    if (a < 64 && b < 64 && c < 64) {
        pa = D_801EA790[0][a];
        pb = D_801EA790[1][b];
        pc = D_801EA790[2][c];
        src = pa;
        t = dst = m->tmp;
        i = m->n;
        do {
            *dst++ = *src++;
        } while (--i != 0);
        src = pb;
        dst = t;
        i = m->n;
        do {
            *dst++ &= *src++;
        } while (--i != 0);
        src = pc;
        dst = t;
        i = m->n;
        do {
            *dst++ &= *src++;
        } while (--i != 0);
        src = t;
        dst = m->bits;
        i = m->n;
        do {
            *dst++ |= *src++;
        } while (--i != 0);
    }
}
