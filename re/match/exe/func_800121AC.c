#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

extern RECT D_80039C60;
extern s32 D_800112C8;
extern void (* volatile D_80049F98)(void);
extern s32 D_80049FA0;
void func_80012048(void);
void func_800165E4(s32, s32, s32);
void func_80011FD8(s32);

void func_800121AC(void)
{
    DISPENV disp;
    DRAWENV draw;

    SetDispMask(0);
    DrawSync(0);
    ClearImage(&D_80039C60, 0, 0, 0);
    func_800165E4(1, 0x65, D_800112C8);
    DrawSync(0);
    func_80011FD8(D_800112C8);
    DrawSync(0);
    SetDefDispEnv(&disp, 0, 0xF0, 0x140, 0xF0);
    SetDefDrawEnv(&draw, 0, 0xF0, 0x140, 0xF0);
    PutDispEnv(&disp);
    PutDrawEnv(&draw);
    D_80049F98 = func_80012048;
    D_80049FA0 = 0;
    SetDispMask(1);
}
