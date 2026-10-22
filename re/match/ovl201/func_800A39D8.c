#include "ac1/ac1.h"
/* ovl201 0x800A39D8 - yes/no prompt update: up/down (with key repeat)
 * toggle the choice byte +0x9E between 0 and 1; X confirms (1: func_80073B7C,
 * then func_800A3D90; 0: func_800A3E5C), triangle cancels (func_800A3D30).
 * Every exit closes object 3 and reopens the box. */
typedef struct {
    u32 used;           /* +0x00 */
    s16 delay;          /* +0x04 */
    s16 rate;           /* +0x06 */
    u8 pad08[0x10];
    u32 press;          /* +0x18 */
    u32 held;           /* +0x1C */
} PadState;
extern PadState D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_800506EC(s32 id);
extern void func_80051450(s32 a, s32 b);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);
extern void func_80073B7C(void);
extern void func_800A3D30(UiObj *o);
extern void func_800A3D90(UiObj *o);
extern void func_800A3E5C(UiObj *o);

typedef struct {
    u8 pad7c[0x22];
    u8 choice;          /* +0x9E */
} YesNoWork;

void func_800A39D8(UiObj *o)
{
    YesNoWork *w = (YesNoWork *)&o->w;

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
        if (w->choice == 0) {
            w->choice = 1;
        } else {
            w->choice--;
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
        if (w->choice == 1) {
            w->choice = 0;
        } else {
            w->choice++;
        }
        return;
    }
    if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        if (w->choice == 1) {
            func_80073B7C();
            func_80051450(4, 7);
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(0x5C, 0x60, 0x88, 0x30, 3);
            o->update = func_800A3D90;
        } else if (w->choice == 0) {
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(0x5C, 0x60, 0x88, 0x30, 3);
            o->update = func_800A3E5C;
        }
    } else if (D_801A2550.press & 0x20) {
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550.used |= 0x20;
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(0x5C, 0x60, 0x88, 0x30, 3);
        o->update = func_800A3D30;
    }
}
