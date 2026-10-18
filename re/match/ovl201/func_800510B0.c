#include "ac1_types.h"
extern s32 D_801F36D0[];
extern s32 D_801BC478[];
void func_800510B0(void)
{
    s16 i;
    for (i = 0; i < 30; i++) {
        D_801F36D0[i] = 0;
        D_801BC478[i] = 0;
    }
}
