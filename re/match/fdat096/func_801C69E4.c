#include "ac1/ac1.h"

typedef struct { s32 w[0x39C / 4]; } Rec;
extern Rec D_80040FA0[];
extern u8 D_80039C5D;
extern s8 D_80039D00;

void func_801C69E4(void)
{
    D_80040FA0[0] = D_80040FA0[1];
    D_80039C5D = 0xFF;
    D_80039D00 = 0;
}
