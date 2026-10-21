#include "ac1/ac1.h"

typedef struct {
    u8 f0;
    u8 pad1;
    u16 f2;
    u16 v[20];
} Rec2C;

extern Rec2C D_8019B4F8[];

void func_80072BDC(u16 *arg0) {
    Rec2C *p;
    u16 *dst;
    s32 i, j;

    p = D_8019B4F8;
    i = 2;
    arg0 += 20;
    for (; i != 0; i--) {
        p->f0 = 0;
        dst = p->v;
        for (j = 20; j != 0; j--) {
            *dst++ = *arg0++;
        }
        p->f2 = p->v[11];
        p++;
    }
}
