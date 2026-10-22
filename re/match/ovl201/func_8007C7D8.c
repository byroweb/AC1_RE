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
    u8 pad7c[4];
    s8 sel;             /* +0x80 cursor row */
} PaintMenuWork;
extern s16 D_801A2318;
extern u8 D_80039CA2;
extern void func_800506EC(s32 a);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 n);
extern void func_8007CBC0(UiObj *o);
extern void func_8007CCE4(UiObj *o);
extern void func_8007CDD8(UiObj *o);
extern void func_8007CECC(UiObj *o);

void func_8007C7D8(UiObj *o)
{
    PaintMenuWork *w = (PaintMenuWork *)&o->w;

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
    } else if (D_801A2550.press & 0x4000) {
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
    }
    if (w->sel < 0) {
        w->sel = 1;
    } else if (w->sel >= 2) {
        w->sel = 0;
    }
    if ((D_801A2550.press & 0x40) && !(D_801A2550.held & 0x40)) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        switch (w->sel) {
        case 0:
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(0x20, 0x28, 0x88, 0x30, 3);
            o->update = func_8007CCE4;
            break;
        case 1:
            if (D_80039CA2 == 0) {
                func_800506EC(4);
                obj_find_by_id(3)->kill = 1;
                func_8005C2D0(0x20, 0x28, 0x88, 0x30, 3);
                o->update = func_8007CDD8;
            } else {
                func_800506EC(4);
                obj_find_by_id(3)->kill = 1;
                func_8005C2D0(0x20, 0x28, 0x88, 0x30, 3);
                o->update = func_8007CECC;
            }
            break;
        case 2:
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(0x20, 0x28, 0x88, 0x30, 3);
            o->update = func_8007CBC0;
            break;
        }
    } else if ((D_801A2550.press & 0x20) && !(D_801A2550.held & 0x20)) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(0x20, 0x28, 0x88, 0x30, 3);
        o->update = func_8007CBC0;
    }
}
