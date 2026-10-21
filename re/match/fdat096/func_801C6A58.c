#include "ac1/ac1.h"

typedef struct {
    s32 w[0x39C / 4];
} Func801C6A58Rec;
extern Func801C6A58Rec D_80040FA0[];

void func_801C6A58(void)
{
    D_80040FA0[1] = D_80040FA0[0];
}
