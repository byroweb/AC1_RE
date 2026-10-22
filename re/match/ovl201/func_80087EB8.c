#include "ac1/ac1.h"

extern s32 D_801F4114[2][64];
extern void func_80087C94(s32 value, char *buf);
extern void func_8005D15C(char *s, s32 x, s32 y);

void func_80087EB8(u8 id, u8 row)
{
    char buf[8];

    func_80087C94(D_801F4114[0][id] + D_801F4114[1][id], buf);
    if (id == 28) {
        buf[0] = '-';
        buf[1] = '-';
        buf[2] = '-';
        buf[3] = '-';
        buf[4] = '-';
        buf[5] = '-';
        buf[6] = 0;
    }
    func_8005D15C(buf, 960, row * 16 + 176);
    DrawSync(0);
}
