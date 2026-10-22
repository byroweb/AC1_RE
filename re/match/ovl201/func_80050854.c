#include "ac1/ac1.h"
extern RECT D_800CF368;
void func_80050854(u32 *p)
{
    u16 *h;
    p++;
    if (*p++ & 8) {
        h = (u16 *)(p + 1);
        p += *p >> 2;
        D_800CF368.x = *h++;
        D_800CF368.y = *h++;
        D_800CF368.w = *h++;
        D_800CF368.h = *h++;
        LoadImage(&D_800CF368, (u32 *)h);
        DrawSync(0);
    }
    h = (u16 *)(p + 1);
    p += *p >> 2;
    D_800CF368.x = *h++;
    D_800CF368.y = *h++;
    D_800CF368.w = *h++;
    D_800CF368.h = *h++;
    LoadImage(&D_800CF368, (u32 *)h);
    DrawSync(0);
}
