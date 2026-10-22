#include "ac1/ac1.h"

/* save/load menu work area (UiObj +0x7C) */
typedef struct {
    UiObj *owner;       /* +0x7C */
    UiObj *arg;         /* +0x80 */
    UiObj *box;         /* +0x84 */
    u8 pad88[0x19];
    s8 menu;            /* +0xA1 system menu cursor 0..4 */
    s8 cursor;          /* +0xA2 row on screen */
    u8 mode;            /* +0xA3 */
    u8 slot;            /* +0xA4 */
    u8 kind;            /* +0xA5 'S' save / 'L' load */
    u8 top;             /* +0xA6 first entry shown */
    u8 unkA7;
    u8 padA8[2];
    u8 what;            /* +0xAA 'D' data / 'E' */
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
extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_800506EC(s32 id);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);
extern void func_80055C2C(UiObj *o);
extern void func_80052F68(UiObj *o);
extern void func_8005700C(UiObj *o);

void func_80052BD0(UiObj *o)
{
    SaveWork *w = (SaveWork *)&o->w;

    if (w->arg == 0) {
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
            if (w->menu == 0) {
                w->menu = 4;
            } else {
                w->menu--;
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
            if (w->menu == 4) {
                w->menu = 0;
            } else {
                w->menu++;
            }
            return;
        }
    }
    if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        w->box->kill = 1;
        func_8005C2D0(0x10, 0x28, 0x80, 0x78, 3);
        func_8005C2D0(0xA0, 0x20, 0x90, 0xA0, 5);
        o->update = func_80055C2C;
    } else if (D_801A2550.press & 0x40) {
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550.used |= 0x40;
        if (w->arg == 0) {
            switch (w->menu) {
            case 0:
                w->kind = 'S';
                w->what = 'D';
                break;
            case 1:
                w->kind = 'L';
                w->what = 'D';
                break;
            case 2:
                w->kind = 'S';
                w->what = 'E';
                break;
            case 3:
                w->kind = 'L';
                w->what = 'E';
                break;
            case 4:
                o->update = func_8005700C;
                return;
            }
            o->update = func_80052F68;
        }
    }
}
