#include "ac1/ac1.h"

extern u32 func_80016A84(s32 port);
extern s32 D_80039D14;
extern u32 D_801A2550;
extern s16 D_801A2554;
extern s16 D_801A2556;
extern s16 D_801A255C;
extern s16 D_801A255E;
extern u32 D_801A2564;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern u8 D_801A2570;

/* per-frame pad poll: key-repeat timers, read pad 1, stir the random seed, edge mask */
void func_8005B4B8(UiObj *o)
{
    u32 pad;
    s32 seed;
    u32 inv;
    s16 *p;

    p = &D_801A2556;
    if (*p != 0) {
        (*p)--;
    } else if (D_801A2554 != 0) {
        D_801A2554--;
    } else {
        D_801A2550 &= 0xFFFF0FFC;
    }
    p = &D_801A255E;
    if (*p != 0) {
        (*p)--;
    } else if (D_801A255C != 0) {
        D_801A255C--;
    } else {
        D_801A2550 &= 0x0FFFFFFF;
    }
    D_801A256C = D_801A2564;
    pad = func_80016A84(1);
    D_801A2564 = pad;
    seed = D_80039D14 + 0x3FD;
    D_80039D14 = ((seed + pad) * 0x6D) & 0x7FFF;
    if (D_801A2570 != 0) {
        D_801A2564 = (pad >> 16) | ((pad & 0xFFFF) << 16);
    }
    inv = ~D_801A2550;
    D_801A2568 = D_801A2564 & inv;
    D_801A2550 = (D_801A2564 | inv) & D_801A2550;
}
