#include "ac1/ac1.h"

/* pad block at 0x801A2550 */
typedef struct {
    u32 used;           /* 0x801A2550 buttons consumed this frame */
    s16 delay1;         /* 0x801A2554 */
    s16 delay2;         /* 0x801A2556 */
    u8 pad08[0x10];
    u32 press;          /* 0x801A2568 */
    u32 held;           /* 0x801A256C */
} PadState;

/* yes/no box (built by func_800ADC4C / func_800ADD70) */
typedef struct {
    UiObj *cursor;      /* +0x7C */
    UiObj *label0;      /* +0x80 */
    UiObj *label1;      /* +0x84 */
    s16 sel;            /* +0x88 0 = yes, 1 = no */
    u8 pad8a[2];
    UiObj *owner;       /* +0x8C */
    UiObj *target;      /* +0x90 */
    s16 n;              /* +0x94 */
    u8 result;          /* +0x96 */
    u8 mode;            /* +0x97 */
} YesNoWork;

extern PadState D_801A2550;
extern s16 D_801A2318;
extern void func_800506EC(s32 a);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 n);
extern void func_800AE128(UiObj *o);

void func_800ADE38(UiObj *o)
{
    YesNoWork *w = (YesNoWork *)&o->w;
    u32 ok;
    u32 cancel;

    if (w->mode != 0) {
        if ((D_801A2550.press & 0x2000) && !(D_801A2550.held & 0x2000) && w->sel == 0) {
            SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
            w->sel = 1;
        } else if ((D_801A2550.press & 0x8000) && !(D_801A2550.held & 0x8000) && w->sel == 1) {
            SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
            w->sel = 0;
        }
    } else {
        if ((D_801A2550.press & 0x20000000) && !(D_801A2550.held & 0x20000000) && w->sel == 0) {
            SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
            w->sel = 1;
        } else if ((D_801A2550.press & 0x80000000) && !(D_801A2550.held & 0x80000000) && w->sel == 1) {
            SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
            w->sel = 0;
        }
    }
    w->cursor->pos.vx = w->sel * 69 + 4;
    if (w->mode != 0) {
        ok = 0x40;
        cancel = 0x20;
    } else {
        ok = 0x400000;
        cancel = 0x200000;
    }
    if (D_801A2550.press & (ok | cancel)) {
        D_801A2550.used |= D_801A2550.press;
        if (D_801A2550.press & cancel) {
            SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
            w->result = 1;
        } else {
            if (w->sel == 0) {
                w->result = 0;
                SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
            } else {
                w->result = 1;
                SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
            }
        }
        func_800506EC(w->n);
        obj_find_by_id(w->n - 1)->kill = 1;
        w->cursor->kill = 1;
        w->label0->kill = 1;
        w->label1->kill = 1;
        func_8005C2D0(0x58, 0x60, 0x90, 0x30, (s16)(w->n - 1));
        o->update = func_800AE128;
    }
}
