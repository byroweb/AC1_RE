#include "ac1/ac1.h"

int LoadImage(RECT *rect, u32 *p);

/* TIM chain loader: every 0x10 TIM goes to VRAM at (x, y), its CLUT
 * (flag bit 3) to (cx, cy). */
void func_80097FDC(u32 *p, s16 x, s16 y, s16 cx, u16 cy)
{
    RECT rect;
    u16 *q;

    while (*p++ == 0x10) {
        if (*p++ & 8) {
            q = (u16 *)(p + 1);
            p += *p >> 2;
            q += 2;
            rect.x = cx;
            rect.y = cy;
            rect.w = *q++;
            rect.h = *q++;
            LoadImage(&rect, (u32 *)q);
        }
        q = (u16 *)(p + 1);
        p += *p >> 2;
        q += 2;
        rect.x = x;
        rect.y = y;
        rect.w = *q++;
        rect.h = *q++;
        LoadImage(&rect, (u32 *)q);
    }
}
