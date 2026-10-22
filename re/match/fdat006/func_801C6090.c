#include "ac1/ac1.h"

extern u16 D_8019F528;
extern s32 D_801C6204;
extern void func_8004C318(s32);
extern void func_800529F0(s32, s32, s32);
extern void func_801C6050(s32);
extern void func_801C6068(s32);

s32 func_801C6090(s32 phase)
{
    s32 v;

    if (D_8019F528 == 0) {
        switch (phase) {
        case 100:
            v = 238;
            goto common;
        case 101:
            v = 237;
common:
            func_801C6068(v);
            D_801C6204++;
            if (D_801C6204 == 2) {
                func_801C6050(221);
                func_801C6050(222);
                func_800529F0(0, 0, 119);
            }
            break;
        case 102:
            func_801C6068(270);
            func_8004C318(128);
            break;
        }
    }
    return 1;
}
