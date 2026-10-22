#include "ac1/ac1.h"
/* ovl201 0x800A57F0 - six-item main menu update (cursor byte +0x8A, 0..5,
 * key repeat on up/down).  X runs the item: 0 checks the save/AC state
 * (func_800714FC) and picks the next mission rank tier from D_80039D14,
 * 1/2 use func_80083A58 results, 3/4 need core/head/arms/legs equipped,
 * 5 goes to func_800A6520; refusals show a message box (func_8005DED8).
 * Triangle opens the exit prompt (func_8005D8D4, func_800A6E54). */
typedef struct {
    u32 used;           /* +0x00 */
    s16 delay;          /* +0x04 */
    s16 rate;           /* +0x06 */
    u8 pad08[0x10];
    u32 press;          /* +0x18 */
    u32 held;           /* +0x1C */
} PadState;
typedef struct {
    u8 pad7c[0xC];
    u8 box[2];          /* +0x88 */
    s8 cursor;          /* +0x8A */
    u8 sub;             /* +0x8B */
} MainMenuWork;

extern PadState D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern u8 D_80031A58;
extern u8 D_80031A5A;
extern u8 D_80031A5B;
extern u8 D_80031A63;
extern char D_80031BD4[];
extern u8 D_80039CAE;
extern s32 D_80039D14;
extern u8 D_800411F8;
extern u8 D_80048510[];
extern s32 D_800485A0;
extern s32 D_800485A4;
extern s32 D_800485F0;
extern s32 D_800485F4;
extern u8 D_80048663;
extern u8 D_8004D29C[];
extern u8 D_8004D2BC[];
extern u8 D_8004D2F0[];
extern u8 D_8004D310[];
extern RECT D_8004D2E0;
extern RECT D_8004D2E8;
extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern int MoveImage(RECT *r, int x, int y);
extern int DrawSync(int mode);
extern void func_80050AF4(s32 a, s32 b);
extern void func_80051450(s32 a, s32 b);
extern void func_8005D8D4(void *def, u8 *dst, s32 a, s32 b);
extern void func_8005DED8(UiObj *o, void (*ret)(UiObj *), void *msg, s32 a);
extern s32 func_800714FC(void *a, char *b);
extern void func_80083A58(void);
extern s32 func_80084290(void);
extern void func_800A576C(void);
extern void func_800A5EA4(UiObj *o);
extern void func_800A6084(UiObj *o);
extern void func_800A63D4(UiObj *o);
extern void func_800A6520(UiObj *o);
extern void func_800A6B88(UiObj *o);
extern void func_800A6E54(UiObj *o);
extern void func_800A7028(UiObj *o);

void func_800A57F0(UiObj *o)
{
    MainMenuWork *w = (MainMenuWork *)&o->w;
    u8 rank;

    if (D_801A2550.press & 0x1000) {
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
            w->cursor = 5;
        } else {
            w->cursor--;
        }
        return;
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
        if (w->cursor == 5) {
            w->cursor = 0;
        } else {
            w->cursor++;
        }
        return;
    }
    if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        switch (((MainMenuWork *)&o->w)->cursor) {
        case 1:
            func_80083A58();
            D_800485F0 = 0;
            D_800485F4 = 0;
            if (D_800485A0 == 0 && D_800485A4 == 0) {
                func_8005DED8(o, func_800A57F0, D_8004D29C, 0xC);
                o->update = ui_obj_nop2;
            } else {
                func_800A576C();
                o->update = func_800A5EA4;
            }
            break;
        case 2:
            func_80083A58();
            if (D_800485F0 == 0) {
                func_8005DED8(o, func_800A57F0, D_8004D2BC, 0xC);
                o->update = ui_obj_nop2;
            } else if (D_800485A0 == 0 && D_800485A4 == 0) {
                func_8005DED8(o, func_800A57F0, D_8004D29C, 0xC);
                o->update = ui_obj_nop2;
            } else {
                RECT a = D_8004D2E0;
                RECT b = D_8004D2E8;

                MoveImage(&a, 0x200, 0x100);
                MoveImage(&b, 0x140, 0x1FF);
                DrawSync(0);
                func_80050AF4(0xD, (u8)func_80084290() + 0x84);
                func_80051450(0x11, (u8)func_80084290());
                func_800A576C();
                o->update = func_800A6084;
            }
            break;
        case 3:
            if (D_80031A58 == 0xFF || D_80031A5A == 0xFF || D_80031A5B == 0xFF || D_80031A63 == 0xFF) {
                func_8005DED8(o, func_800A57F0, D_8004D2F0, 0xC);
                o->update = ui_obj_nop2;
            } else {
                func_800A576C();
                o->update = func_800A63D4;
            }
            break;
        case 4:
            if (D_80031A58 == 0xFF || D_80031A5A == 0xFF || D_80031A5B == 0xFF || D_80031A63 == 0xFF) {
                func_8005DED8(o, func_800A57F0, D_8004D2F0, 0xC);
                o->update = ui_obj_nop2;
            } else {
                func_800A576C();
                o->update = func_800A6B88;
            }
            break;
        case 5:
            ((MainMenuWork *)&o->w)->sub = 0;
            o->update = func_800A6520;
            break;
        case 0:
            if (!(u8)func_800714FC(D_80048510, D_80031BD4)) {
                func_8005DED8(o, func_800A57F0, D_8004D2F0, 0xC);
                o->update = ui_obj_nop2;
                break;
            }
            if (D_80048663 != 2) {
                if ((D_80039D14 & 0xFF) >= 205) {
                    rank = 90;
                } else if ((D_80039D14 & 0xFF) >= 154) {
                    rank = 91;
                } else if ((D_80039D14 & 0xFF) >= 103) {
                    rank = 92;
                } else if ((D_80039D14 & 0xFF) >= 52) {
                    rank = 93;
                } else {
                    rank = 94;
                }
                D_80039CAE = rank;
            } else {
                D_80039CAE++;
                if (D_80039CAE == 95) {
                    D_80039CAE = 90;
                }
                rank = D_80039CAE;
            }
            D_800411F8 = rank;
            func_80051450(4, (u16)(rank - 38));
            func_800A576C();
            o->update = func_800A7028;
            break;
        }
    } else if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        func_8005D8D4(D_8004D310, ((MainMenuWork *)&o->w)->box, 0xC, 1);
        o->update = func_800A6E54;
    }
}
