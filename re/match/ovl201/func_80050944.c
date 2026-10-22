#include "ac1/ac1.h"
extern RECT D_800CF370;
void func_80050944(u32 *p)
{
    u16 *h;
    while (*p++ == 0x10) {
        if (*p++ & 8) {
            h = (u16 *)(p + 1);
            p += *p >> 2;
            D_800CF370.x = *h++;
            D_800CF370.y = *h++;
            D_800CF370.w = *h++;
            D_800CF370.h = *h++;
            LoadImage(&D_800CF370, (u32 *)h);
            DrawSync(0);
        }
        h = (u16 *)(p + 1);
        p += *p >> 2;
        D_800CF370.x = *h++;
        D_800CF370.y = *h++;
        D_800CF370.w = *h++;
        D_800CF370.h = *h++;
        LoadImage(&D_800CF370, (u32 *)h);
        DrawSync(0);
    }
}
