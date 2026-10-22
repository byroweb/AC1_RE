#include "ac1/ac1.h"
/* ovl201 0x8005F49C - copy a 4 x 16 VRAM block from (x + sx, sy + 256) to
 * (dx, dy), keeping only the 4-bit pixels of one colour plane (kind 240..243
 * picks the nibble mask). */

extern u32 D_8017E520[32];
extern int StoreImage(RECT *rect, u32 *p);
extern int LoadImage(RECT *rect, u32 *p);
extern int DrawSync(int mode);

void func_8005F49C(s32 x, s16 kind, s16 dx, s16 dy, s32 sx, s32 sy)
{
    RECT r;
    u32 mask;
    s16 i;

    r.w = 4;
    r.h = 16;
    r.x = x + sx;
    r.y = sy + 256;
    StoreImage(&r, D_8017E520);
    DrawSync(0);
    switch (kind) {
    case 240:
        mask = 0x11111111;
        break;
    case 241:
        mask = 0x22222222;
        break;
    case 242:
        mask = 0x44444444;
        break;
    case 243:
        mask = 0x88888888;
        break;
    }
    for (i = 0; i < 32; i++) {
        D_8017E520[i] &= mask;
    }
    r.x = dx;
    r.y = dy;
    LoadImage(&r, D_8017E520);
}
