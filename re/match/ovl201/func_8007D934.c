#include "ac1/ac1.h"
/* ovl201 0x8007D934 - update of the paint part-group menu: up/down move the 6-row cursor
 * (wraps), confirm on rows 0..4 picks the part group (G/H/C/A/B) and opens the colour
 * editor (update 0x8007DF2C), row 5 restores the default colours 0x8004C4F8; cancel
 * closes the box and returns to the caller object (or 0x8007DE1C) */

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
    s8 r, g, b;
} Rgb3;

typedef struct {
    Rgb3 c[4][4];
} Palette;

typedef struct {
    UiObj *caller;              /* +0x7C object to hand control back to */
    void (*ret)(UiObj *);       /* +0x80 its update on return */
    s8 sel;                     /* +0x84 cursor row, 5 = default colours */
    u8 part;                    /* +0x85 'G','H','C','A','B' */
} PaintPartWork;

extern s16 D_801A2318;
extern Palette D_8004C4F8;
extern Rgb3 D_80031A64[4][4];
extern void func_800506EC(s32 a);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 n);
extern void func_80073DDC(void);
extern void func_8007DE1C(UiObj *o);
extern void func_8007DF2C(UiObj *o);

void func_8007D934(UiObj *o)
{
    PaintPartWork *w = (PaintPartWork *)&o->w;
    Palette def;
    s16 i;
    s16 j;

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
        w->sel = 5;
    } else if (w->sel >= 6) {
        w->sel = 0;
    }
    if ((D_801A2550.press & 0x40) && !(D_801A2550.held & 0x40)) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        switch (w->sel) {
        case 0:
            w->part = 'G';
            o->update = func_8007DF2C;
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(32, 40, 136, 144, 3);
            return;
        case 1:
            w->part = 'H';
            o->update = func_8007DF2C;
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(32, 40, 136, 144, 3);
            return;
        case 2:
            w->part = 'C';
            o->update = func_8007DF2C;
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(32, 40, 136, 144, 3);
            return;
        case 3:
            w->part = 'A';
            o->update = func_8007DF2C;
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(32, 40, 136, 144, 3);
            return;
        case 4:
            w->part = 'B';
            o->update = func_8007DF2C;
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(32, 40, 136, 144, 3);
            return;
        case 5:
            def = D_8004C4F8;
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    D_80031A64[i][j] = def.c[i][j];
                }
            }
            func_80073DDC();
            return;
        }
    }
    if ((D_801A2550.press & 0x20) && !(D_801A2550.held & 0x20)) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(32, 40, 136, 144, 3);
        if (w->caller == &g_ui_none) {
            o->update = func_8007DE1C;
        } else {
            w->caller->update = w->ret;
            o->kill = 1;
        }
    }
}
