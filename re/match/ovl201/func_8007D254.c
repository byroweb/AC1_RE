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
    UiObj *caller;              /* +0x7C object to hand control back to */
    void (*ret)(UiObj *);       /* +0x80 its update on return */
    s8 sel;                     /* +0x84 cursor row, 4 = back */
} PaintColorWork;

extern s16 D_801A2318;
extern u8 D_80039CA2;
extern s8 D_80039CA3;
extern u8 D_8004C4B4[];
extern void func_800506EC(s32 a);
extern void func_80051450(s32 a, u16 b);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 n);
extern void func_8005D15C(void *p, s32 x, s32 y);
extern void func_8007D640(UiObj *o);
extern void func_8007DE1C(UiObj *o);

void func_8007D254(UiObj *o)
{
    PaintColorWork *w = (PaintColorWork *)&o->w;

    if (D_801A2550.press & 0x5000) {
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
            w->sel--;
            SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
            if (w->sel < 0) {
                w->sel = 3;
            }
        } else {
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
            w->sel++;
            SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
            if (w->sel >= 4) {
                w->sel = 0;
            }
        }
    }
    if ((D_801A2550.press & 0x40) && !(D_801A2550.held & 0x40)) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        if (w->sel != 4) {
            D_80039CA3 = w->sel;
            func_80051450(4, w->sel + (D_80039CA2 * 4 + 58));
            o->update = func_8007D640;
        } else {
            func_8005D15C(D_8004C4B4, 960, 128);
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(0x20, 0x28, 0x88, 0x80, 3);
            if (w->caller == &g_ui_none) {
                o->update = func_8007DE1C;
            } else {
                w->caller->update = w->ret;
                o->kill = 1;
            }
        }
    } else if ((D_801A2550.press & 0x20) && !(D_801A2550.held & 0x20)) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        func_8005D15C(D_8004C4B4, 960, 128);
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(0x20, 0x28, 0x88, 0x90, 3);
        if (w->caller == &g_ui_none) {
            o->update = func_8007DE1C;
        } else {
            w->caller->update = w->ret;
            o->kill = 1;
        }
    }
}
