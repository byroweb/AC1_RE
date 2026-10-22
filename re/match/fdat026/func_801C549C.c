#include "ac1/ac1.h"

extern s32 func_80052338();
extern void func_80078CFC(void *);
extern void func_8008A80C();
extern void func_8008C2FC(s32);
extern s32 func_801C5418(s32);
extern u16 D_8019F528;
extern u8 D_8019FB94[];
extern u8 D_8019FBC0[];
extern s32 D_801CB0CC;
extern s32 D_801CB0D0;

void func_801C549C(void)
{
    s32 d5;

    if (D_801CB0D0 == 0) {
        func_8008C2FC(0x601);
        D_801CB0D0 = 1;
    }
    func_8008A80C();
    if (D_8019F528 == 0 && D_801CB0CC == 1 && func_80052338() != 0) {
        d5 = func_801C5418(5);
        if (func_801C5418(6) < d5) func_80078CFC(D_8019FB94); else func_80078CFC(D_8019FBC0);
        D_801CB0CC = 2;
    }
}
