#include "ac1/ac1.h"

typedef struct LetterWork {
    u8 pad00[0x0B];
    volatile s8 tim_slot;       /* +0x0B pending TIM upload 1..3; polled by 0x80098268 */
} LetterWork;

typedef struct { LetterWork *work; } LetterCtl;
extern LetterCtl D_80180860;

void func_80097FDC(u32 *p, s16 x, s16 y, s16 cx, u16 cy);

void func_800980E4(u32 *tim)
{
    switch (D_80180860.work->tim_slot) {
    case 1:
        func_80097FDC(tim, 896, 0, 0, 481);
        break;
    case 2:
        func_80097FDC(tim, 960, 0, 0, 482);
        break;
    case 3:
        func_80097FDC(tim, 896, 128, 0, 483);
        break;
    }
    D_80180860.work->tim_slot = 0;
    DrawSync(0);
    free2(tim);
}
