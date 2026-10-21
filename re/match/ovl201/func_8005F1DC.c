#include "ac1/ac1.h"

extern void func_80050AF4(s32, s32);
extern void func_80051450(s32, s32);
extern void func_80074A18(void);
extern s32 func_8007AA04(s32);
extern u8 D_80031A59;

void func_8005F1DC(u8 flag)
{
    u8 cur;
    s32 v;

    cur = D_80031A59;
    if (cur == 0xFF) {
        v = func_8007AA04(1);
        if ((u8)v != cur) {
            func_80050AF4(14, (v & 0xFF) + 64);
            func_80051450(11, (u8)v);
        }
    }
    if (flag) {
        func_80074A18();
    }
}
