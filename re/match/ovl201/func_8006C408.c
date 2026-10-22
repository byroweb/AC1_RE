#include "ac1/ac1.h"

typedef struct EmbImage {
    u8 pad[0xA];
    u8 color;           /* +0x0A pen colour (palette index) */
    u8 padB;
    u8 *pixels;         /* +0x0C 4bpp 64x64, 32 bytes per row */
} EmbImage;

void func_8006C408(EmbImage *img, s32 x, s32 y)
{
    u8 *p;

    if ((u16)x < 64 && (u16)y < 64) {
        if (x & 1) {
            p = &img->pixels[(s16)x / 2 + (s16)y * 32];
            *p &= 0xF;
            *p |= img->color << 4;
        } else {
            p = &img->pixels[(s16)x / 2 + (s16)y * 32];
            *p &= 0xF0;
            *p |= img->color & 0xF;
        }
    }
}
