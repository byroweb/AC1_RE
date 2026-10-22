#include "ac1/ac1.h"

typedef struct {
    s8 b0;
    s8 b1;
    s8 b2;
    s8 b3;
    s8 b4;
    s8 b5;
    s16 h6;
    s16 h8;
    s16 arr[15];
    s32 w28;
    s32 w2c;
    s32 w30;
    s32 w34;
    s8 flags[64];
} State3750;

extern s8 D_801A2308;
extern State3750 D_801F3750;
extern s8 D_801F37C8;
extern s8 D_801F37C9;
extern s8 D_801F37CA;
extern s8 D_801F37D4[64];

void func_80086EC8(void)
{
    s32 k = 64;
    s32 j = 64;
    s32 i = 15;

    D_801F3750.b0 = 0;
    D_801F3750.b1 = -1;
    D_801F3750.b2 = 0;
    D_801F3750.b3 = 0;
    D_801F3750.b4 = 1;
    D_801F3750.b5 = 0;
    D_801F3750.h6 = 0;
    D_801F3750.h8 = 0;
    while (--i != -1) {
        D_801F3750.arr[i] = 0;
    }
    D_801F3750.w2c = 1;
    D_801F3750.w28 = 0;
    D_801F3750.w30 = 0;
    D_801F3750.w34 = 0;
    while (--j != -1) {
        D_801F3750.flags[j] = 0;
    }
    D_801F37C8 = 0;
    D_801F37C9 = 0;
    D_801F37CA = 0;
    while (--k != -1) {
        D_801F37D4[k] = -1;
    }
    D_801A2308 = 0;
}
