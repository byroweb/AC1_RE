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
extern u16 D_801A257C;
extern u8 D_800B6FC8;
extern u8 D_800B91AC[];
extern u8 D_800B91B8[];
extern u8 D_8004B148[];
extern u8 D_8004B168[];
extern u8 D_8004B190[];
extern u8 D_8004B1AC[];
extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_800506EC(s32 id);
extern UiObj *func_8005CAE0(void *p, s32 x, s32 y, s32 n);
extern s32 func_800685A8(u8 slot);
extern void func_80051C44(u8 *p, s32 c);
extern void func_80053E98(UiObj *o);
extern void func_8005D8D4(void *msg, u8 *out, s32 a, s32 b);
extern void func_8005489C(UiObj *o);
extern void func_80054AB0(UiObj *o);
extern void func_80054F1C(UiObj *o);
extern void func_80055580(UiObj *o);

void func_800542F0(UiObj *o)
{
    SaveWork *w = (SaveWork *)&o->w;
    u8 total;
    u8 rows;

    if (D_800B6FC8 != 0) {
        w->arrow_up = func_8005CAE0(D_800B91AC, 0x94, 0x30, 2);
        w->arrow_up->light = 0xB;
        w->arrow_up->visible = 0;
        w->arrow_down = func_8005CAE0(D_800B91B8, 0x94, 0xD4, 2);
        w->arrow_down->light = 0xB;
        w->arrow_down->visible = 0;
        D_800B6FC8 = 0;
    }
    if (func_800685A8(w->slot) != 0) {
        D_800B6FC8 = 1;
        w->arrow_up->kill = 1;
        w->arrow_down->kill = 1;
        o->update = func_8005489C;
    }
    total = D_801A257C;
    rows = total;
    if (w->new_row != 0) {
        total++;
        rows = total;
    }
    if (rows > 5) {
        rows = 5;
    }
    if (D_800B6FC8 == 0) {
        if (w->top != 0) {
            w->arrow_up->visible = 1;
        } else {
            w->arrow_up->visible = 0;
        }
        if (total - w->top > 5) {
            w->arrow_down->visible = 1;
        } else {
            w->arrow_down->visible = 0;
        }
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
        if (w->cursor == rows - 1) {
            if (total - w->top > 5) {
                w->top++;
                func_800506EC(8);
                func_80051C44((u8 *)&w->cursor, 'L');
                func_80051C44((u8 *)&w->cursor, 'R');
                func_80053E98(o);
            }
        } else {
            w->cursor++;
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
        if (w->cursor == 0) {
            if (w->top != 0) {
                w->top--;
                func_800506EC(8);
                func_80051C44((u8 *)&w->cursor, 'L');
                func_80051C44((u8 *)&w->cursor, 'R');
                func_80053E98(o);
            }
        } else {
            w->cursor--;
        }
    } else if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        if (w->arg == 0) {
            D_800B6FC8 = 1;
            w->arrow_up->kill = 1;
            w->arrow_down->kill = 1;
            o->update = func_8005489C;
        } else {
            D_800B6FC8 = 1;
            w->arrow_up->kill = 1;
            w->arrow_down->kill = 1;
            o->update = func_8005489C;
        }
    } else if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        if (w->kind == 'S') {
            if (w->top + w->cursor < D_801A257C) {
                func_8005D8D4(D_8004B148, &w->mode, 10, 1);
            } else {
                func_8005D8D4(D_8004B168, &w->mode, 10, 0);
            }
            D_800B6FC8 = 1;
            w->arrow_up->kill = 1;
            w->arrow_down->kill = 1;
            o->update = func_80054AB0;
        } else if (w->top + w->cursor < D_801A257C) {
            func_8005D8D4(D_8004B190, &w->mode, 10, 0);
            D_800B6FC8 = 1;
            w->arrow_up->kill = 1;
            w->arrow_down->kill = 1;
            o->update = func_80054F1C;
        } else {
            func_8005D8D4(D_8004B1AC, &w->mode, 10, 0);
            D_800B6FC8 = 1;
            w->arrow_up->kill = 1;
            w->arrow_down->kill = 1;
            o->update = func_80055580;
        }
    }
}
