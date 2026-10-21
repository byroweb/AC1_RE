#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

extern void func_8004D4C8(void);
extern s32 func_800162A0(s32);
extern void func_80057288(void);

void func_800571F0(void) {
    DrawSync(0);
    func_8004D4C8();
    while (func_800162A0(D_801AC810.rcnt) < 715) {
    }
    PutDispEnv(&D_801A5F1C.frame->disp);
    PutDrawEnv(&D_801A5F1C.frame->draw);
    DrawOTag(D_801A5F1C.ctx.ot + 0x1FFF);
    D_801AC810.rcnt = GetRCnt(0xF2000001);
    func_80057288();
}
