#include "ac1/ac1.h"

extern void *D_801A5F34[];
extern s32 func_80015D80();
extern void func_800165E4(u16, u16, u8 *);
extern void func_800574D8(u8 *);

s32 func_8005360C(u8 *p, u16 x, u16 y, s32 slot) {
    s32 r;

    D_801A5F34[slot] = p;
    r = func_80015D80(x, y);
    func_800165E4(x, y, p);
    func_800574D8(p + *(s32 *)(p + 8));
    return r;
}
