#include "ac1/ac1.h"
#include "ac1/gfxframe.h"

typedef struct {                /* ovl204 display buffer */
    u32 *ot;                    /* +0x00 */
    u8 pad04[4];
    DRAWENV draw;               /* +0x08 */
    u8 pad64[0x11C - 0x64];
    DISPENV disp;               /* +0x11C */
} Gfx4Frame;

typedef struct {
    Gfx4Frame *frame;
} Gfx4State;

extern Gfx4State D_801ADCC4;
extern s32 D_801B4108;

extern void func_8004D5C8(void);
extern s32 func_800162A0(s32);
extern void func_800562FC(void);

void func_8005625C(void) {
    DrawSync(0);
    func_8004D5C8();
    while (func_800162A0(D_801B4108) < 715) {
    }
    PutDispEnv(&D_801ADCC4.frame->disp);
    PutDrawEnv(&D_801ADCC4.frame->draw);
    DrawOTag(D_801ADCC4.frame->ot + 0x1FFF);
    D_801B4108 = GetRCnt(0xF2000001);
    func_800562FC();
}
