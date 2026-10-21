#include "ac1/ac1.h"

extern RECT D_800B7408;
extern int MoveImage(RECT *rect, int x, int y);

void func_8005D21C(u8 *s, s16 x, s16 y) {
    while (*s != 0) {
        D_800B7408.x = ((*s - 32) & 0x1F) * 2 + 448;
        D_800B7408.y = ((*s - 32) / 32) * 8 + 48;
        MoveImage(&D_800B7408, x, y);
        s++;
        x += 2;
    }
}
