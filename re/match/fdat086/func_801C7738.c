#include "ac1/ac1.h"

extern void func_800529F0();
extern void func_8008A80C();
extern s32 D_80198BB8;
extern s32 D_80198BEC;
extern u16 D_8019F528;
extern s32 D_801CD4C0;

void func_801C7738(void)
{
    func_8008A80C();
    if (D_8019F528 == 0) {
        if (D_801CD4C0 == 0x294) {
            func_800529F0(0, 0, 0x56);
        } else if (D_801CD4C0 == 0x344) {
            D_80198BB8 = 0;
            D_80198BEC = 0;
        } else if (D_801CD4C0 == 0x370) {
            func_800529F0(0, 0, 0x57);
        }
        if (D_801CD4C0 < 0xFFFF) {
            D_801CD4C0++;
        }
    }
}
