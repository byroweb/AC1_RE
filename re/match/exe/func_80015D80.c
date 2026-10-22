#include "ac1/ac1.h"

typedef struct {
    u16 *tab;
    u8 pad4[8];
} Func80015D80Ent;     /* 0x0C */
extern Func80015D80Ent D_8004A2A4[];

s32 func_80015D80(u16 a, u16 b)
{
    u16 *p;
    s32 lo, hi;

    p = D_8004A2A4[a].tab;
    lo = p[b];
    hi = p[b + 1];
    return (hi - lo) << 11;
}
