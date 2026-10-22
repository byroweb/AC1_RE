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
    u8 pad7c[0xD];
    u8 sel;             /* +0x89 option 0..2 */
} OptWork;

extern PadState D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern u8 D_80048663;
extern void func_800506EC(s32 a);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 n);
extern void func_800AFA00(UiObj *o);

void func_800AFBFC(UiObj *o)
{
    OptWork *w = (OptWork *)&o->w;

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
            w->sel = 2;
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
        if (w->sel != 2) {
            w->sel++;
        } else {
            w->sel = 0;
        }
    } else if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_8005C2D0(0xA8, 0x40, 0x88, 0x48, 7);
        o->update = func_800AFA00;
    } else if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        D_80048663 = w->sel;
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_8005C2D0(0xA8, 0x40, 0x88, 0x48, 7);
        o->update = func_800AFA00;
    }
}
