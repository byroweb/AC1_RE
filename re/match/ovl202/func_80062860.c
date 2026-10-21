#include "ac1/ac1.h"

typedef struct {
    u8 c[4];
} Col4;

extern Col4 D_8008DCA0[][32];
extern Col4 D_80090A28[][32];

/* func_8006280C with each channel scaled (4.12) and lifted by 64, clamped to 255 */
void func_80062860(s32 row, s32 first, s32 n, s32 scale)
{
    Col4 *d;
    Col4 *s;
    s32 v;

    d = &D_80090A28[row][first];
    s = &D_8008DCA0[row][first];
    do {
        v = ((s->c[0] * scale) >> 12) + 64;
        if (v >= 256) {
            v = 255;
        }
        d->c[0] = v;
        v = ((s->c[1] * scale) >> 12) + 64;
        if (v >= 256) {
            v = 255;
        }
        d->c[1] = v;
        v = ((s->c[2] * scale) >> 12) + 64;
        if (v >= 256) {
            v = 255;
        }
        d->c[2] = v;
        d++;
        s++;
    } while (--n);
}
