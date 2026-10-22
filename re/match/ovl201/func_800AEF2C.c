#include "ac1/ac1.h"

/* pad block at 0x801A2550 */
typedef struct {
    u32 used;           /* 0x801A2550 buttons consumed this frame */
    s16 delay1;         /* 0x801A2554 first repeat delay */
    s16 delay2;         /* 0x801A2556 */
    u8 pad08[0x10];
    u32 press;          /* 0x801A2568 */
    u32 held;           /* 0x801A256C */
} PadState;

typedef struct {
    UiObj *src;         /* +0x7C */
    u8 pad80[8];
    u8 sel;             /* +0x88 menu row 0..5 */
} OptMenuWork;

extern PadState D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern void func_80052134(UiObj *o);
extern void func_800AE838(void);
extern void func_800AEEDC(void);
extern void func_800AF374(UiObj *o);
extern void func_800AF4AC(UiObj *o);
extern void func_800AF5CC(UiObj *o);
extern void func_800AFA50(UiObj *o);
extern void func_800AFEB4(s32 kind, UiObj *o, void (*back)(UiObj *));
extern void func_800B1A80(UiObj *o);

void func_800AEF2C(UiObj *o)
{
    OptMenuWork *w;

    func_800AEEDC();
    w = (OptMenuWork *)&o->w;
    if (D_801A2550.press & 0x1000) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        D_801A2550.used |= 0x1000;
        if (D_801A2574 != 0) {
            D_801A2550.delay1 = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.delay2 = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay1 = 6;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.delay2 = 12;
            }
        }
        if (w->sel != 0) {
            w->sel--;
        } else {
            w->sel = 5;
        }
    } else if (D_801A2550.press & 0x4000) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        D_801A2550.used |= 0x4000;
        if (D_801A2574 != 0) {
            D_801A2550.delay1 = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.delay2 = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay1 = 6;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.delay2 = 12;
            }
        }
        if (w->sel != 5) {
            w->sel++;
        } else {
            w->sel = 0;
        }
    } else if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        switch (w->sel) {
        case 0:
            SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
            func_800AFEB4('M', o, func_800AEF2C);
            o->update = ui_obj_nop2;
            break;
        case 1:
            SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
            o->update = ui_obj_nop2;
            func_800AFEB4('B', o, func_800AEF2C);
            break;
        case 2:
            if (w->src->ctor != func_80052134) {
                SsUtKeyOn(D_801A2318, 0, 7, 60, 0, 127, 127);
            } else {
                SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
                o->update = func_800AF5CC;
            }
            break;
        case 3:
            SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
            o->update = func_800AFA50;
            break;
        case 4:
            SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
            o->update = func_800AF374;
            break;
        case 5:
            SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
            o->update = func_800B1A80;
            break;
        }
    } else if (D_801A2550.press & 0x20) {
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        D_801A2550.used |= 0x20;
        func_800AE838();
        o->update = func_800AF4AC;
    }
}
