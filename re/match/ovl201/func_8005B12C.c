#include "ac1/ac1.h"

extern int MoveImage(RECT *rect, int x, int y);
extern int DrawSync(int mode);
extern RECT D_8017E518;
extern RECT D_800B7374;
extern RECT D_800B737C;
extern u8 D_80031A94[];
extern u16 D_80039CA0;
extern u8 D_80031A58;
extern u8 D_80031A5B;
extern u8 D_80031A5C;
extern u8 D_80031A5D;

void func_8005B12C(void)
{
    s16 i;
    s32 m;
    s32 t;

    D_8017E518.x = 0x2C0;
    D_8017E518.y = 0;
    D_8017E518.w = 0x80;
    D_8017E518.h = 0x100;
    MoveImage(&D_8017E518, 0x2C0, 0x100);
    D_8017E518.x = 0x230;
    D_8017E518.y = 0x180;
    D_8017E518.w = 0x10;
    D_8017E518.h = 0x40;
    MoveImage(&D_8017E518, 0x220, 0x180);
    D_8017E518.x = 0;
    D_8017E518.y = 0x1EF;
    D_8017E518.w = 0x100;
    D_8017E518.h = 9;
    MoveImage(&D_8017E518, 0, 0x1E6);
    MoveImage(&D_800B7374, 0x180, 0x100);
    MoveImage(&D_800B737C, 0x140, 0x1E2);
    DrawSync(0);
    m = 0;
    i = 0x56;
    do {
        t = (m << 1) | D_80031A94[i];
        i--;
        m = t;
    } while (i >= 0x4C);
    D_80039CA0 &= t;
    if (D_80031A58 == 0xFF) {
        D_80039CA0 = 0;
    }
    if (D_80031A5B == 0xFF) {
        D_80031A5D = 0xFF;
        D_80031A5C = 0xFF;
    }
}
