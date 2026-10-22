#include "ac1/ac1.h"
/* ovl202 0x8008AA90: mission end sequence step (end timer running) */
extern s16 func_800132CC(s16, s32, s32);
extern s32 func_80052338(void);
extern void func_8008A9FC(void);
extern s16 D_800411C0;
extern s16 D_800411C2;
extern u32 D_800411F4[];
extern u32 D_8019F524[];
extern s32 D_801AC80C[];

void func_8008AA90(void)
{
    s16 a;

    D_801AC80C[0] = 0;
    D_800411F4[0] &= ~0x10000;
    D_8019F524[0] |= 0x400;
    a = func_800132CC(D_800411C0, 0, 8);
    D_800411C0 = a;
    if (a > 0x800) {
        D_800411C0 = a - 0x1000;
    }
    D_800411C2 = 0;
    if (g_mission_end_timer == 24 && func_80052338() == 0) {
        g_mission_end_timer++;
    }
    func_8008A9FC();
}
