#include "ac1/ac1.h"

/* pad block at 0x801A2550 */
typedef struct {
    u32 used;           /* 0x801A2550 buttons consumed this frame */
    s16 delay1;         /* 0x801A2554 first repeat delay */
    s16 delay2;         /* 0x801A2556 */
    u8 pad08[0x10];
    u32 press;          /* 0x801A2568 */
    u32 held;           /* 0x801A256C */
    u8 pad20[4];
    s32 speed;          /* 0x801A2574 */
} PadState;

extern PadState D_801A2550;

typedef struct {
    u8 pad7c[8];
    UiObj *up;          /* +0x84 scroll arrow up */
    UiObj *down;        /* +0x88 scroll arrow down */
    u8 sel;             /* +0x8C selected pattern 0..15 */
    u8 top;             /* +0x8D first row shown */
} PatternWork;

extern s16 D_801A2318;
extern u8 D_80039CA2;
extern void func_80073DDC(void);
extern void func_8005BA2C(s32 x, s32 y, s32 w, s32 h, s32 a, s32 b, s32 c);
extern void func_80080B80(UiObj *o);
extern void func_80080CA8(UiObj *o);
extern void func_80080EB0(UiObj *o);
extern void func_80080F40(void);

void func_800807CC(UiObj *o)
{
    PatternWork *w = (PatternWork *)&o->w;

    if (w->top == 0) {
        w->up->visible = 0;
    } else {
        w->up->visible = 1;
    }
    if (w->top == 9) {
        w->down->visible = 0;
    } else {
        w->down->visible = 1;
    }
    if (D_801A2550.press & 0x1000) {
        D_801A2550.used |= 0x1000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 6 / D_801A2550.speed;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 6;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.delay2 = 12;
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (w->sel != 0) {
            w->sel--;
        }
        if (w->sel < w->top) {
            w->top--;
            func_80080EB0(o);
        }
    }
    if (D_801A2550.press & 0x4000) {
        D_801A2550.used |= 0x4000;
        if (D_801A2550.speed != 0) {
            D_801A2550.delay1 = 6 / D_801A2550.speed;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.delay2 = 12 / D_801A2550.speed;
            }
        } else {
            D_801A2550.delay1 = 6;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.delay2 = 12;
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (w->sel < 15) {
            w->sel++;
        }
        if (w->top + 6 < w->sel) {
            w->top++;
            func_80080EB0(o);
        }
    }
    if ((D_801A2550.press & 0x40) && !(D_801A2550.held & 0x40)) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        if (D_80039CA2 != w->sel) {
            D_80039CA2 = w->sel;
            func_80080F40();
            if (w->sel == 0) {
                func_80073DDC();
            }
            func_8005BA2C(16, 120, 280, 32, 9, 1, 0);
            o->update = func_80080B80;
        }
    } else if ((D_801A2550.press & 0x20) && !(D_801A2550.held & 0x20)) {
        D_801A2550.used |= D_801A2550.press;
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        w->up->kill = 1;
        w->down->kill = 1;
        o->update = func_80080CA8;
    }
}
