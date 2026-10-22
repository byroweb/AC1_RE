#include "ac1/ac1.h"

typedef struct {
    s32 cur;            /* 0x80040FA0 */
    s32 prev;           /* 0x80040FA4 */
    s32 last;           /* 0x80040FA8 */
    s32 last2;          /* 0x80040FAC */
    u8 pad10[0x262];
    u16 paused;         /* 0x80041212 */
} FrameClock;

extern FrameClock D_80040FA0;
extern s32 D_80040FA8;
extern s32 func_80016A84(s32 mode);

void func_80079524(void)
{
    s32 t;

    D_80040FA0.prev = D_80040FA0.cur;
    t = func_80016A84(1);
    D_80040FA0.cur = t;
    D_80040FA0.last2 = D_80040FA0.last;
    if (!(g_mission_flags & 0x400) && D_80040FA0.paused != 0) {
        D_80040FA0.last = t;
    } else {
        D_80040FA8 = 0;
    }
}
