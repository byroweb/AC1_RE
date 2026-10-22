#include "ac1/ac1.h"

extern s32 rand(void);
extern s32 func_80014D78(s32);
extern void func_8004C318(s32);
extern s32 func_80052338(u16 *);
extern void func_800529F0(s32, s32, s32);
extern void func_8008A798();
typedef struct {
    u8 state;           /* +0x00 */
    u8 pad1[9];
    s16 link;           /* +0x0A -1 = none */
    u8 pad0c[0x20];
} Slot2C;               /* 0x2C */
extern Slot2C D_8019FBEC;
extern Slot2C D_8019FC18;
extern Slot2C D_8019FC44;
extern Slot2C D_8019FC70;
extern void func_8008C2FC(s32);
extern u16 D_8019F528;
extern u16 D_8019F52A;
extern s16 D_8019F52E;
extern s16 D_8019F532;
extern s32 D_801AC80C;
extern s32 D_801C7224;
extern s16 D_801C7218;
extern s16 D_801C721C;
extern s32 D_801C7220;

void func_801C6E54(void)
{
    s32 n;
    s16 *px;
    s16 *py;
    u16 *pf;

    if (rand() < 328) {
        D_801C7218 = func_80014D78(257) - 128;
        D_801C721C = func_80014D78(257) - 128;
    }
    px = &D_8019F52E;
    if (*px < D_801C7218) {
        *px = *px + 1;
    } else if (D_801C7218 < *px) {
        *px = *px - 1;
    }
    py = &D_8019F532;
    if (*py < D_801C721C) {
        *py = *py + 1;
    } else if (D_801C721C < *py) {
        *py = *py - 1;
    }
    n = D_801C7220;
    if (n < 4096) {
        D_801C7220 = n + 1;
        func_8008A798(n);
    }
    pf = &D_8019F52A;
    if (*pf == 0xFFFE) {
        *pf = 0;
        D_801AC80C = 1;
    }
    if (D_8019F528 == 0 && D_8019FBEC.link != -1 && D_8019FBEC.state == 3
        && D_8019FC18.link != -1 && D_8019FC18.state == D_8019FBEC.state
        && D_8019FC44.link != -1 && D_8019FC44.state == D_8019FC18.state
        && D_8019FC70.link != -1 && D_8019FC70.state == D_8019FC44.state) {
        if (D_801C7224 == 10) {
            func_800529F0(0, 0, 104);
            func_8004C318(128);
        }
        D_801C7224++;
    }
}
