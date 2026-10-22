#include "ac1/ac1.h"

/* save/load menu work area (UiObj +0x7C) */
typedef struct {
    UiObj *owner;       /* +0x7C */
    UiObj *arg;         /* +0x80 */
    UiObj *box;         /* +0x84 */
    UiObj *icon[2];     /* +0x88 per card slot */
    UiObj *arrow_up;    /* +0x90 */
    UiObj *arrow_down;  /* +0x94 */
    s32 status[2];      /* +0x98 per card slot */
    s8 pick;            /* +0xA0 slot cursor 0..1 */
    s8 menu;            /* +0xA1 */
    s8 cursor;          /* +0xA2 row on screen */
    u8 mode;            /* +0xA3 */
    u8 slot;            /* +0xA4 */
    u8 kind;            /* +0xA5 'S' save / 'L' load */
    u8 top;             /* +0xA6 first entry shown */
    u8 new_row;         /* +0xA7 */
    u8 col;             /* +0xA8 grid column 0..3 */
    u8 row;             /* +0xA9 grid row 0..1 */
} SaveWork;

typedef struct {
    u32 used;           /* +0x00 buttons consumed */
    s16 delay;          /* +0x04 repeat delay */
    s16 rate;           /* +0x06 repeat rate */
    u8 pad8[0x10];
    u32 press;          /* +0x18 */
    u32 held;           /* +0x1C */
} PadState;
extern PadState D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern u8 D_8004B190[];
extern u8 D_8004B324[];
extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern s32 func_800685A8(u8 slot);
extern void func_8005D8D4(void *msg, u8 *out, s32 a, s32 b);
extern void func_800566AC(UiObj *o);
extern void func_800567B8(UiObj *o);
extern void func_80056B60(UiObj *o);

void func_8005616C(UiObj *o)
{
    SaveWork *w = (SaveWork *)&o->w;

    if (func_800685A8(w->slot) != 0) {
        o->update = func_800566AC;
    }
    if (D_801A2550.press & 0x4000) {
        D_801A2550.used |= 0x4000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.rate = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.rate = 12;
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w->col != 3) {
            if (w->row == 1) {
                w->row = 0;
            } else {
                w->row = 1;
            }
        }
    } else if (D_801A2550.press & 0x1000) {
        D_801A2550.used |= 0x1000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.rate = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.rate = 12;
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w->col != 3) {
            if (w->row == 0) {
                w->row = 1;
            } else {
                w->row = 0;
            }
        }
    } else if (D_801A2550.press & 0x2000) {
        D_801A2550.used |= 0x2000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.rate = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.rate = 12;
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w->row == 1) {
            if (w->col == 2) {
                w->col = 0;
            } else {
                w->col++;
            }
        } else {
            if (w->col == 3) {
                w->col = 0;
            } else {
                w->col++;
            }
        }
    } else if (D_801A2550.press & 0x8000) {
        D_801A2550.used |= 0x8000;
        if (D_801A2574 != 0) {
            D_801A2550.delay = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.rate = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.rate = 12;
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (w->row == 1) {
            if (w->col == 0) {
                w->col = 2;
            } else {
                w->col--;
            }
        } else {
            if (w->col == 0) {
                w->col = 3;
            } else {
                w->col--;
            }
        }
    } else if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        if (w->arg == 0) {
            o->update = func_800566AC;
        } else {
            o->update = func_800566AC;
        }
    } else if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        if (w->kind == 'S') {
            func_8005D8D4(D_8004B324, &w->mode, 10, 1);
            o->update = func_800567B8;
        } else {
            func_8005D8D4(D_8004B190, &w->mode, 10, 0);
            o->update = func_80056B60;
        }
    }
}
