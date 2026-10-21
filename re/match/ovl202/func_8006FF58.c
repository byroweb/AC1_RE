#include "ac1/ac1.h"
/* ovl202 0x8006FF58 - is p inside triangle abc: the three edge sides all agree */
void func_8006FF34(void *a, void *b, void *p, s32 *out);

s32 func_8006FF58(void *p, void *a, void *b, void *c)
{
    s32 s0;
    s32 s1;
    s32 s2;
    s32 r;

    func_8006FF34(a, b, p, &s0);
    func_8006FF34(b, c, p, &s1);
    func_8006FF34(c, a, p, &s2);
    r = 0;
    if ((s0 >= 0 && s1 >= 0 && s2 >= 0) || (s0 <= 0 && s1 <= 0 && s2 <= 0)) {
        r = 1;
    }
    return r;
}
