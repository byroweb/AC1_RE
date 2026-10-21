#include "ac1/ac1.h"

extern s32 func_8001628C(void);
extern void func_8004D948(s32 a);
extern void func_8004DB18(u16 id);
extern u16 D_80041BBE;
extern u8 D_80041BC1;
extern u8 D_80041BC2;
extern s32 D_80041BC4;

void func_8004D974(void)
{
    u16 *cur = &D_80041BBE;

    if (*cur != 0xFFFF && D_80041BC2 == 0 && D_80041BC4 - func_8001628C() < 0) {
        if (!(D_80041BC1 & 1)) {
            func_8004DB18(*cur);
        } else {
            D_80041BC2 = 0xF0;
            func_8004D948(0);
        }
    }
}
