#include "ac1/ac1.h"

typedef struct {
    SVECTOR v;
    u16 a;                      /* +0x08 */
    u8 b;                       /* +0x0A */
    u8 c;                       /* +0x0B */
    u8 d;                       /* +0x0C */
    u8 e;                       /* +0x0D */
    u8 pad0e[2];
} Rec16;

extern Rec16 D_801AD82C[];

void func_80056AD0(u8 *src) {
    Rec16 *r;
    u8 *s;
    s32 i;

    r = D_801AD82C;
    i = 256;
    s = src;
    do {
        r->v = *(SVECTOR *)s;
        s += 8;
        r->a = *(u16 *)s;
        s += 2;
        r->b = *s++;
        r->c = *s++;
        r->d = *s;
        r->e = 0;
        s += 2;
        r++;
    } while (--i);
}
