#include "ac1/ac1.h"

typedef struct {                /* UiObj +0x7C: optional-parts shop list */
    UiObj *owner;               /* +0x00 */
    void (*back)(UiObj *);      /* +0x04 owner->update restored on exit */
    UiObj *arrow_up;            /* +0x08 */
    UiObj *arrow_down;          /* +0x0C */
    u8 pad10[0x34];
    s32 w44;                    /* +0x44 */
    u8 row;                     /* +0x48 cursor row on screen */
    u8 rows;                    /* +0x49 */
    u8 sel;                     /* +0x4A entry at the cursor */
    u8 loading;                 /* +0x4B */
    u8 b4C;                     /* +0x4C */
    u8 b4D;                     /* +0x4D */
    u8 b4E;                     /* +0x4E */
    u8 b4F;                     /* +0x4F */
    u8 b50;                     /* +0x50 */
} OpWork;

typedef struct {
    u32 used;           /* +0x00 buttons consumed */
    s16 delay;          /* +0x04 repeat delay */
    s16 rate;           /* +0x06 repeat rate */
    u8 pad8[0x10];
    u32 press;          /* +0x18 */
    u32 held;           /* +0x1C */
} PadState;

extern PadState D_801A2550;
extern struct { s32 v; } D_801A2574;
extern s16 D_801A2318;
extern u16 D_80039CA0;
extern u8 D_80031A58;
extern u8 D_800B9482[][56];
extern u8 D_800BB11C[][44];
extern RECT D_800B8A5C;
extern RECT D_800B8A64;
extern u8 D_8004CA50[];
extern void *D_801F3704;
extern void *D_801F36D0[];
extern void *D_801F36D4;
extern void *D_801F36D8;
extern s32 D_801BC4AC;
extern s32 D_801BC47C;
extern s32 D_801BC480;
extern void func_800845F8(void);
extern s32 func_800517C8(void);
extern void func_800788C8(s32 a, u8 b);
extern s32 func_80085258(u8 a);
extern void func_80050AF4(s16 i, s16 id);
extern void func_80051450(s32 a, u8 b);
extern s32 func_80084290(void);
extern s32 func_80084314(u8 sel);
extern u8 func_8008423C(u8 sel);
extern u8 func_800841E8(u8 sel);
extern void func_80084670(UiObj *o);
extern s32 func_80084364(UiObj *o);
extern void func_80084884(UiObj *o);
extern void func_8005DED8(UiObj *o, void (*f)(UiObj *), u8 *msg, s32 n);
extern void func_80051628(void);
extern void func_800506EC(s32 id);
extern void func_80051078(void *p);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);

void func_80084AE4(UiObj *o)
{
    OpWork *w = (OpWork *)&o->w;
    s32 k;
    u16 bit;
    u16 f;
    u8 left;
    s16 i;
    s16 a;
    u8 t;
    u8 one;

    func_800845F8();
    if (w->loading != 0 && (s16)func_800517C8() == 0) {
        w->loading = 0;
        func_800788C8(13, w->b4C);
        w->w44 = func_80085258(w->b4D);
        w->b4D = 255;
    } else if (w->b4E != 0 && (s16)func_800517C8() == 0) {
        if (w->b4C == 1) {
            a = 2;
        } else {
            a = 1;
        }
        w->b4C = a;
        func_80050AF4(a, w->b4F);
        func_80051450(17, w->b50);
        w->b4E = 0;
        w->loading = 1;
    }
    if (w->sel != (u8)func_80084290()) {
        w->arrow_up->visible = 1;
    } else {
        w->arrow_up->visible = 0;
    }
    if ((u8)func_80084314(w->sel) >= 5) {
        w->arrow_down->visible = 1;
    } else {
        w->arrow_down->visible = 0;
    }
    if (D_801A2550.press & 0x1000) {
        D_801A2550.used |= 0x1000;
        if (D_801A2574.v != 0) {
            D_801A2550.delay = 6 / D_801A2574.v;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.rate = 12 / D_801A2574.v;
            }
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.held & 0x1000)) {
                D_801A2550.rate = 12;
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (w->row != 0) {
            w->row--;
            func_80084670(o);
        } else if (w->sel != (u8)func_80084290()) {
            w->sel = func_8008423C(w->sel);
            func_80084670(o);
        }
        return;
    }
    if (D_801A2550.press & 0x4000) {
        D_801A2550.used |= 0x4000;
        if (D_801A2574.v != 0) {
            D_801A2550.delay = 6 / D_801A2574.v;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.rate = 12 / D_801A2574.v;
            }
        } else {
            D_801A2550.delay = 6;
            if (!(D_801A2550.held & 0x4000)) {
                D_801A2550.rate = 12;
            }
        }
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (w->row != w->rows - 1) {
            if (w->row == 4) {
                if ((u8)func_80084314(w->sel) >= 5) {
                    w->sel = func_800841E8(w->sel);
                    func_80084670(o);
                }
            } else {
                w->row++;
                func_80084670(o);
            }
        }
        return;
    }
    if (D_801A2550.press & 0x40) {
        k = func_80084364(o);
        D_801A2550.used |= 0x40;
        bit = 1 << k;
        if (D_80039CA0 & bit) {
            D_80039CA0 &= ~bit;
            SsUtKeyOn(D_801A2318, 0, 11, 60, 0, 127, 127);
        } else {
            f = D_80039CA0;
            left = D_800B9482[D_80031A58][0];
            for (i = 0; i < 11; i++) {
                if (f & 1) {
                    left -= D_800BB11C[i][0x28];
                }
                f >>= 1;
            }
            if (left < D_800BB11C[(u8)k][0x28]) {
                func_8005DED8(o, func_80084AE4, D_8004CA50, 12);
                o->update = ui_obj_nop2;
                return;
            }
            D_80039CA0 |= bit;
            SsUtKeyOn(D_801A2318, 0, 10, 60, 0, 127, 127);
        }
        func_80084884(o);
        return;
    }
    if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        w->arrow_up->kill = 1;
        w->arrow_down->kill = 1;
        if ((s16)func_800517C8() != 0) {
            func_80051628();
        }
        while ((s16)func_800517C8() != 0) {
        }
        MoveImage(&D_800B8A5C, 384, 256);
        MoveImage(&D_800B8A64, 320, 482);
        DrawSync(0);
        func_800506EC(4);
        one = 1;
        obj_find_by_id(3)->kill = one;
        func_800506EC(6);
        obj_find_by_id(5)->kill = one;
        func_800506EC(8);
        obj_find_by_id(7)->kill = one;
        func_800506EC(10);
        obj_find_by_id(9)->kill = one;
        if (D_801F36D0[13] != 0) {
            func_80051078(D_801F36D0[13]);
            D_801F36D0[13] = 0;
            D_801BC4AC = 0;
        }
        if (D_801F36D4 != 0) {
            func_80051078(D_801F36D4);
            D_801F36D4 = 0;
            D_801BC47C = 0;
        }
        if (D_801F36D8 != 0) {
            func_80051078(D_801F36D8);
            D_801F36D8 = 0;
            D_801BC480 = 0;
        }
        func_8005C2D0(16, 64, 144, 120, 3);
        func_8005C2D0(168, 34, 128, 120, 5);
        func_8005C2D0(168, 158, 128, 40, 7);
        func_8005C2D0(16, 202, 288, 24, 9);
        D_801A2574.v = 2;
        w->owner->update = w->back;
        o->kill = one;
    }
}
