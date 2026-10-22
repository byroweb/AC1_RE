#include "ac1/ac1.h"

typedef struct {
    s32 now;                    /* +0x00 */
    s32 old;                    /* +0x04 */
    s32 x;                      /* +0x08 */
    s32 xold;                   /* +0x0C */
    u8 unk10[0x39C - 0x10];
} PadRec;

extern s32 func_80016A84(s32 port);
extern PadRec D_80040FA0[2];
extern s32 D_801AB684;

void func_80077C30(void)
{
    s32 v;

    v = func_80016A84(1);
    D_80040FA0[0].old = D_80040FA0[0].now;
    D_80040FA0[0].now = v;
    D_80040FA0[1].old = D_80040FA0[1].now;
    D_80040FA0[1].now = v >> 16;
    D_80040FA0[0].xold = D_80040FA0[0].x;
    D_80040FA0[1].xold = D_80040FA0[1].x;
    if (!(D_801AB684 & 0x400)) {
        D_80040FA0[0].x = v;
        D_80040FA0[1].x = v >> 16;
    } else {
        D_80040FA0[1].x = 0;
        D_80040FA0[0].x = 0;
    }
}
