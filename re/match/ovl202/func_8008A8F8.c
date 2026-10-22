#include "ac1/ac1.h"

extern void func_8004D178(s32, s32, s32, s32, s32);
extern void func_8004DB18(u8);
extern s32 func_80052338(void);
extern u8 D_80039C5D;
/* g_mission_end_seq accessed as an array: retail keeps its address in a
 * register (la form) */
extern u16 D_8019F52A[];

void func_8008A8F8(void)
{
    if (D_8019F52A[0] == 60) {
        g_mission_flags &= ~0x1000;
        if (D_80039C5D != 0xFF && func_80052338() != 0) {
            func_8004DB18(D_80039C5D);
            return;
        }
        D_8019F52A[0]--;
        return;
    }
    g_mission_flags |= 0x1000;
    if (D_8019F52A[0] == 20) {
        func_8004D178(38, 0, 80, 80, 0);
    }
    if (D_8019F52A[0] == 5) {
        func_8004D178(39, 0, 100, 100, 0);
    }
}
