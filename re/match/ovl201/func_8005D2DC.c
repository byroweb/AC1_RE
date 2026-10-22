#include "ac1/ac1.h"

extern RECT D_800B7410;

/* Blit a string from the 8x8 font page in VRAM: one MoveImage per glyph. */
void func_8005D2DC(u8 *s, s16 x, s16 y) {
    while (*s != 0) {
        D_800B7410.x = ((*s - 0x20) & 0x1F) * 2 + 0x1C0;
        D_800B7410.y = ((*s - 0x20) / 32) * 8 + 0x60;
        MoveImage(&D_800B7410, x, y);
        s++;
        x++;
    }
}
