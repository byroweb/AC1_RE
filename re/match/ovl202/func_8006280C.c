#include "ac1/ac1.h"

typedef struct {
    u8 c[4];
} Col4;

extern Col4 D_8008DCA0[][32];
extern Col4 D_80090A28[][32];

/* copy n colours of row `row` from the base table into the live table */
void func_8006280C(s32 row, s32 first, s32 n)
{
    Col4 *d;
    Col4 *s;

    d = &D_80090A28[row][first];
    s = &D_8008DCA0[row][first];
    do {
        *d++ = *s++;
    } while (--n);
}
