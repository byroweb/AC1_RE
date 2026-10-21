#include "ac1/ac1.h"

extern s32 D_8009079C;
extern s16 D_801987FC[];
extern s32 D_801AC80C[];
extern u32 D_8019F524[];
extern u16 D_8019F52A[];
extern void func_800529F0(s32 a, s32 b, s32 c);
extern void func_8004D178(s32 id, s32 key, s32 vl, s32 vr, s32 pan);

void func_8008A80C(void)
{
    if (D_8009079C < 37) {
        if (D_8009079C == 36) {
            D_8019F52A[0] = 0;
            D_801AC80C[0] = 1;
            D_8019F524[0] &= ~0x2400;
        } else {
            D_801987FC[0] = 0;
            D_801987FC[1] = 0;
            D_801AC80C[0] = 0;
            D_8019F52A[0] = 0xFFFF;
            D_8019F524[0] |= 0x2400;
            if (D_8009079C == 0) {
                func_800529F0(0, 0, 101);
            }
            if (D_8009079C == 22) {
                func_8004D178(35, 0, 80, 80, 0);
            }
        }
        D_8009079C++;
    }
}
