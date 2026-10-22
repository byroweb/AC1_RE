#include "ac1/ac1.h"

extern void func_8004CFC8();
extern void func_801C6CCC();
extern u16 D_801D0B52;

void func_801C6D94(void)
{
    s32 t;

    t = 0x8C - D_801D0B52;
    if (t == 10 || t == 80) {
        func_8004CFC8(7, 0x7F);
    }
    if (t >= 10 && t < 41) {
        func_801C6CCC(0x11);
    }
    if (t >= 80 && t < 111) {
        func_801C6CCC(0x10);
    }
}
