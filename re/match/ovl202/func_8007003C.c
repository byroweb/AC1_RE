#include "ac1/ac1.h"
/* ovl202 0x8007003C - is p inside quad a b d c: the four edge sides all agree
 * (func_8006FF58 for quads) */
void func_8006FF34(void *a, void *b, void *p, s32 *out);

s32 func_8007003C(void *p, void *a, void *b, void *c, void *d)
{
    s32 s[4];
    s32 r;

    func_8006FF34(a, b, p, &s[0]);
    func_8006FF34(b, d, p, &s[1]);
    func_8006FF34(d, c, p, &s[2]);
    func_8006FF34(c, a, p, &s[3]);
    r = 0;
    if ((s[0] >= 0 && s[1] >= 0 && s[2] >= 0 && s[3] >= 0) || (s[0] <= 0 && s[1] <= 0 && s[2] <= 0 && s[3] <= 0)) {
        r = 1;
    }
    return r;
}
