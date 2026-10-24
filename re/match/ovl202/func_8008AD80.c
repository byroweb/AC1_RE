#include "ac1/ac1.h"
/* ovl202 0x8008AD80: draw a right-aligned signed number with digit sprites (8 px pitch) */
typedef struct {
    SprInfo spr;
    s16 pad[2];
} SprCell;
extern SprCell D_800907A0[];
extern u32 D_80090920;
int func_8008ACE4(char *buf, int n);
void func_80060B54(DVECTOR *xy, SprInfo *spr, s32 rot, u32 *rgb, s32 z, s32 abr);

void func_8008AD80(int x, int y, int n)
{
    DVECTOR xy;
    u8 buf[16];
    u8 *p;
    int len;

    len = func_8008ACE4(buf, n);
    p = buf;
    x += (9 - len) * 8;
    while (len > 0) {
        xy.vx = x;
        xy.vy = y;
        func_80060B54(&xy, &D_800907A0[*p].spr, 0, &D_80090920, 0, 0);
        x += 8;
        len--;
        p++;
    }
}
