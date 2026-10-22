#include "ac1/ac1.h"

extern void func_800529F0();
extern void func_8008A798(s32);
extern u16 D_8019F528;
extern u16 D_8019F52A;
extern u8 D_8019FC70;
extern s32 D_801AC80C;
extern s32 D_801CE570;
extern s32 D_801CE574;
extern s32 D_801CE578;

void func_801C72B4(void)
{
    s32 t;
    u8 u;
    u16 *p;

    t = D_801CE578;
    if (t < 0x1000) {
        D_801CE578 = t + 1;
        func_8008A798(t);
    }
    p = &D_8019F52A;
    if (*p == 0xFFFE) {
        *p = 0;
        D_801AC80C = 1;
    }
    if (D_8019F528 == 0) {
        if (D_801CE574 == 0 && *p == 0) {
            func_800529F0(0, 0, 0x75);
            D_801CE574 = 1;
        }
        if (D_801CE570 == 0) {
            u = D_8019FC70;
            if (u == 1) {
                func_800529F0(0, 0, 0xA0);
                D_801CE570 = u;
            }
        }
    }
}
