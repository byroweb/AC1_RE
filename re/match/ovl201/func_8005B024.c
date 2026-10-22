#include "ac1/ac1.h"

extern int MoveImage(RECT *rect, int x, int y);
extern int DrawSync(int mode);
extern RECT D_8017E510;
extern RECT D_800B7364;
extern RECT D_800B736C;

void func_8005B024(void)
{
    D_8017E510.x = 0x2C0;
    D_8017E510.y = 0x100;
    D_8017E510.w = 0x80;
    D_8017E510.h = 0x100;
    MoveImage(&D_8017E510, 0x2C0, 0);
    D_8017E510.x = 0x220;
    D_8017E510.y = 0x180;
    D_8017E510.w = 0x10;
    D_8017E510.h = 0x40;
    MoveImage(&D_8017E510, 0x230, 0x180);
    D_8017E510.x = 0;
    D_8017E510.y = 0x1E6;
    D_8017E510.w = 0x100;
    D_8017E510.h = 9;
    MoveImage(&D_8017E510, 0, 0x1EF);
    MoveImage(&D_800B7364, 0x200, 0x100);
    MoveImage(&D_800B736C, 0x140, 0x1FF);
    DrawSync(0);
}
