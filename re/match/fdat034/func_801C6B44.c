#include "ac1/ac1.h"

typedef struct {
    u8 st;
    u8 pad1[0x2B];
} Slot;
extern Slot D_8019FAB8[];
extern void func_80078CFC();
extern s32 func_801C6AE0();

void func_801C6B44(s32 a0, s32 a1, s32 a2)
{
    Slot *s;
    s32 d;
    Slot *minp;
    s32 min;
    Slot *maxp;
    s32 max;
    s32 r;
    s32 i;
    Slot *p;

    minp = 0;
    min = 0x7FFFFFFF;
    max = -1;
    a0 >>= 2;
    a0 = a0 * a0;
    s = &D_8019FAB8[a1];
    maxp = 0;
    for (i = 0; i < a2; i++, s++) {
        if (s->st == 0) {
            d = func_801C6AE0(s);
            if (a0 < d && d < min) {
                min = d;
                minp = s;
            }
            if (max < d) {
                max = d;
                maxp = s;
            }
        }
    }
    if (minp != 0) {
        func_80078CFC(minp);
    } else if (maxp != 0) {
        func_80078CFC(maxp);
    }
}
