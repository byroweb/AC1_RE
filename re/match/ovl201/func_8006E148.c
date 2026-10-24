/* ovl201 emblem_image_scroll: scroll the 64x64 4bpp emblem image (+0x88)
 * one pixel with wrap-around.  U/D move 32-byte rows through a row buffer,
 * L/R shift each row's 8 words by one nibble with the carry from the
 * neighbouring word. */
#include "ac1/ac1.h"

void func_8006E148(UiObj *o, u8 dir)
{
    u8 buf[32];
    u32 w[8];
    u8 *img;
    u8 *p;
    s16 i;
    s16 j;
    u32 t;

    img = *(u8 **)((u8 *)o + 0x88);
    if (dir == 'U') {
        p = img;
        for (i = 0; i < 32; i++)
            buf[i] = *p++;
        p = img;
        img = img + 32;
        for (i = 0; i < 2016; i++)
            *p++ = *img++;
        for (i = 0; i < 32; i++)
            *p++ = buf[i];
    } else if (dir == 'D') {
        p = img + 2016;
        for (i = 0; i < 32; i++)
            buf[i] = *p++;
        p = img + 2047;
        img = img + 2015;
        for (i = 0; i < 2016; i++)
            *p-- = *img--;
        for (i = 31; i >= 0; i--)
            *p-- = buf[i];
    } else if (dir == 'L') {
        p = img;
        for (j = 0; j < 64; j++) {
            for (i = 0; i < 8; i++) {
                t = *((u32 *)p)++;
                w[i] = t >> 4;
            }
            for (i = 6; i >= 0; i--) {
                p -= 4;
                w[i] |= *(u32 *)p << 28;
            }
            p -= 4;
            w[7] |= *(u32 *)p << 28;
            for (i = 0; i < 8; i++)
                *((u32 *)p)++ = w[i];
        }
    } else if (dir == 'R') {
        p = img;
        for (j = 0; j < 64; j++) {
            for (i = 0; i < 8; i++) {
                t = *((u32 *)p)++;
                w[i] = t << 4;
            }
            p -= 4;
            t = *(u32 *)p >> 28;
            w[0] |= t;
            for (i = 7; i > 0; i--) {
                p -= 4;
                t = *(u32 *)p >> 28;
                w[i] |= t;
            }
            for (i = 0; i < 8; i++)
                *((u32 *)p)++ = w[i];
        }
    }
}
