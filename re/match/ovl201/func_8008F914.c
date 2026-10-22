#include "ac1/ac1.h"
extern signed char D_8019C092;
extern u16 D_8019C0EA[];
extern u8 D_8019C24C[];
extern s32 D_80039CA4;
extern s32 func_8008F08C(u8 kind, u8 idx);

s32 func_8008F914(void)
{
    s16 i;

    i = D_8019C0EA[0] + D_8019C0EA[D_8019C092 + 30];
    return D_80039CA4 < func_8008F08C(D_8019C092, D_8019C24C[i]);
}
