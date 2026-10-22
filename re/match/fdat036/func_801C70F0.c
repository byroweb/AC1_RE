#include "ac1/ac1.h"

extern void func_8004DB18();
extern s32 func_80052338();
extern void func_8008A80C();
extern void func_8008C2FC(s32);
extern u16 D_8019FBA8;
extern u16 D_8019FBD4;
extern s32 D_801C7360;
extern s32 D_801C7364;
extern s32 D_801C7368;

void func_801C70F0(void)
{
    if (D_801C7360 == 0) {
        func_8008C2FC(0);
        D_801C7360 = 1;
    }
    if (D_801C7364 != 0) {
        if (D_801C7364 == 0x528) {
            u16 *p = &D_8019FBA8;
            *p &= 0xFFBF;
            D_8019FBD4 &= 0xFFBF;
        }
        D_801C7364++;
    }
    if (D_801C7368 == 1 && D_801C7364 != 0 && func_80052338() != 0) {
        func_8004DB18(5);
        D_801C7368 = 2;
    }
    func_8008A80C();
}
