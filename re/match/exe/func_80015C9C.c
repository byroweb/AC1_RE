#include "ac1/ac1.h"

typedef struct {
    u16 *tab;
    u8 pos[8];
} FileEnt;     /* 0x0C */

extern FileEnt D_8004A2A4[];
extern void func_80015A08(void *pos, u16 sec);

s32 func_80015C9C(u16 file, u16 idx, s32 seek)
{
    FileEnt *e;
    u16 lo;

    e = &D_8004A2A4[file];
    lo = e->tab[idx];
    if (seek != 0) {
        func_80015A08(e->pos, lo);
    }
    return (e->tab[idx + 1] - lo) << 11;
}
