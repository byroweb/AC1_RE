#include "ac1/ac1.h"

extern void func_8004C318();
extern void func_800529F0();
extern void func_801C7254();
extern u16 D_8019F528;
extern u8 D_801C7494[], D_801C749C[], D_801C74A4[], D_801C74AC[], D_801C74B4[];
extern s32 D_801C74BC;

s32 func_801C72B4(u32 a0)
{
    u8 *p;
    s32 i;
    s32 snd;
    s32 t;

    if (D_8019F528 == 0) {
        switch (a0) {
        case 0:
            func_801C7254(D_801C7494, 0x1F);
            func_800529F0(0, 0, 0x2D);
            return 1;
        case 1:
            p = D_801C749C;
            i = 0x25;
        cnt:
            func_801C7254(p, i);
            t = ++D_801C74BC;
            if (t == 1) {
                func_800529F0(0, 0, 0x4B);
            } else if (t == 3) {
                func_800529F0(0, 0, 0x4C);
            } else if (t == 4) {
                func_800529F0(0, 0, 0x94);
                func_8004C318(0x100);
            }
            return 1;
        case 2:
            p = D_801C74A4;
            i = 0x26;
            goto cnt;
        case 3:
            p = D_801C74AC;
            i = 0x27;
            goto cnt;
        case 4:
            p = D_801C74B4;
            i = 0x28;
            goto cnt;
        }
    }
    return 0;
}
