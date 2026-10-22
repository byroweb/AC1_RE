#include "ac1/ac1.h"

extern s32 printf(const char *fmt, ...);
extern char D_8004DC50[];
extern char D_8004DC78[];
extern char D_8004DCB0[];
extern volatile u32 *D_800BE31C;
extern volatile u32 *D_800BE324;
extern volatile u32 *D_800BE328;
extern volatile u32 *D_800BE330;
extern volatile u32 *D_800BE350;

/* MDEC timeout: dump the DMA and MDEC status bits, then reset */
s32 func_800B54B4(char *who)
{
    u32 st;

    printf(D_8004DCB0, who);
    st = *D_800BE350;
    printf(D_8004DC50, (*D_800BE324 >> 24) & 1, (*D_800BE330 >> 24) & 1, *D_800BE31C, *D_800BE328);
    printf(D_8004DC78, ~st >> 31, (st >> 30) & 1, (st >> 29) & 1, (st >> 28) & 1, (st >> 27) & 1,
           (st >> 25) & 1, (st >> 23) & 1);
    *D_800BE350 = 0x80000000;
    *D_800BE324 = 0;
    *D_800BE330 = 0;
    *D_800BE330;
    *D_800BE350 = 0x60000000;
    return 0;
}
