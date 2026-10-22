#include "ac1/ac1.h"

extern void func_800B5248(void *tbl, u32 mode);
extern s32 printf(const char *fmt, ...);
extern char D_8004DC14[];
extern u8 D_800BE214[];
extern u8 D_800BE298[];
extern volatile u32 *D_800BE324;
extern volatile u32 *D_800BE330;
extern volatile u32 *D_800BE350;

/* MDEC reset: 0 = reset and reload both tables, 1 = reset only */
void func_800B514C(s32 mode)
{
    switch (mode) {
    case 0:
        *D_800BE350 = 0x80000000;
        *D_800BE324 = 0;
        *D_800BE330 = 0;
        *D_800BE350 = 0x60000000;
        func_800B5248(D_800BE214, 32);
        func_800B5248(D_800BE298, 32);
        break;
    case 1:
        *D_800BE350 = 0x80000000;
        *D_800BE324 = 0;
        *D_800BE330 = 0;
        *D_800BE330;
        *D_800BE350 = 0x60000000;
        break;
    default:
        printf(D_8004DC14, mode);
        break;
    }
}
