#include "ac1/ac1.h"

extern void func_80051450(s32, s32);
extern void func_80074A18(void);
extern s32 func_8007AA04(s32);
extern u8 D_80031A58;
extern u8 D_80031A59;
extern u8 D_80031A5A;
extern u8 D_80031A5B;
extern u8 D_80031A63;
extern u8 D_80039CA2;

/* func_8005F1DC for all five settings */
void func_8005F054(u8 flag)
{
    u8 cur;
    s32 v;

    cur = D_80031A5A;
    if (cur == 0xFF) {
        v = func_8007AA04(2);
        if ((u8)v != cur) {
            func_80051450(10, (v & 0xFF) + D_80039CA2 * 10);
        }
    }
    cur = D_80031A58;
    if (cur == 0xFF) {
        v = func_8007AA04(0);
        if ((u8)v != cur) {
            func_80051450(7, (v & 0xFF) + D_80039CA2 * 3);
        }
    }
    cur = D_80031A5B;
    if (cur == 0xFF) {
        v = func_8007AA04(3);
        if ((u8)v != cur) {
            func_80051450(9, (v & 0xFF) + D_80039CA2 * 16);
        }
    }
    cur = D_80031A63;
    if (cur == 0xFF) {
        v = func_8007AA04(11);
        if ((u8)v != cur) {
            func_80051450(8, (v & 0xFF) + D_80039CA2 * 32);
        }
    }
    cur = D_80031A59;
    if (cur == 0xFF) {
        v = func_8007AA04(1);
        if ((u8)v != cur) {
            func_80051450(11, (u8)v);
        }
    }
    if (flag) {
        func_80074A18();
    }
}
