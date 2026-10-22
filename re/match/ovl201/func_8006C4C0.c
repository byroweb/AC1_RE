#include "ac1/ac1.h"

typedef struct EmbImage {
    u8 pad[0xA];
    u8 color;           /* +0x0A pen colour (palette index) */
    u8 padB;
    u8 *pixels;         /* +0x0C 4bpp 64x64, 32 bytes per row */
} EmbImage;

u8 func_8006C4C0(EmbImage *img, s32 x, s32 y)
{
    if ((u16)x < 64 && (u16)y < 64) {
        if (x & 1) {
            return img->pixels[(s16)x / 2 + (s16)y * 32] >> 4;
        } else {
            return img->pixels[(s16)x / 2 + (s16)y * 32] & 0xF;
        }
    }
}
