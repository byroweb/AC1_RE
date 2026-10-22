#include "ac1/ac1.h"

extern s32 rand(void);
extern s32 func_80014D78(s32);
extern void func_8004C318(s32);
extern s32 func_80052338(u16 *);
extern void func_800529F0(s32, s32, s32);
extern void func_8008A798();
extern void func_8008C2FC(s32);
extern u16 D_8019F528;
extern u16 D_8019F52A;
extern s16 D_8019F52E;
extern s16 D_8019F532;
extern s32 D_801AC80C;
extern s32 D_801C7DE4;
extern s16 D_801C7DE8;
extern s16 D_801C7DEC;
extern s32 D_801C7DF0;
extern s32 D_801C7DF4;

void func_801C79D4(void)
{
    s32 n;
    s16 *px;
    s16 *py;
    u16 *pf;

    if (rand() < 328) {
        D_801C7DE8 = func_80014D78(257) - 128;
        D_801C7DEC = func_80014D78(257) - 128;
    }
    px = &D_8019F52E;
    if (*px < D_801C7DE8) {
        *px = *px + 1;
    } else if (D_801C7DE8 < *px) {
        *px = *px - 1;
    }
    py = &D_8019F532;
    if (*py < D_801C7DEC) {
        *py = *py + 1;
    } else if (D_801C7DEC < *py) {
        *py = *py - 1;
    }
    n = D_801C7DF0;
    if (n < 4096) {
        D_801C7DF0 = n + 1;
        func_8008A798(n);
    }
    pf = &D_8019F52A;
    if (*pf == 0xFFFE) {
        *pf = 0;
        D_801AC80C = 1;
    }
    if (D_8019F528 == 0 && func_80052338(pf) != 0) {
        switch (D_801C7DE4) {
        case 1:
            func_8008C2FC(0);
            D_801C7DE4 = 2;
            break;
        case 2:
            if (D_801C7DF4 == 10) {
                func_800529F0(0, 0, 104);
                func_8004C318(128);
            }
            D_801C7DF4++;
            break;
        }
    }
}
