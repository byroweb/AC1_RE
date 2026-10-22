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

typedef struct {
    u8 col;             /* +0x7C */
    u8 row;             /* +0x7D */
} PadCfgWork;

extern PadState D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern s32 D_80048630[12];      /* config: pad mapping */
extern char D_800BDE18[12][12]; /* button names */
extern UiObj *D_8017F4D8[12];   /* button name labels */
extern UiObj *D_8017F508[12];   /* action labels */
extern void func_800B0CF0(UiObj *o);

/* Pad config: the pressed button becomes the mapping of the selected action; clashes light up. */
void func_800B146C(UiObj *o)
{
    PadCfgWork *w = (PadCfgWork *)&o->w;
    s16 i;
    s16 j;
    s16 k;

    i = w->row + w->col * 8;
    if (D_801A2550.press & 0x1000) {
        D_80048630[i] = 0x1000;
        D_8017F4D8[i]->res = D_800BDE18[0];
    } else if (D_801A2550.press & 0x4000) {
        D_80048630[i] = 0x4000;
        D_8017F4D8[i]->res = D_800BDE18[1];
    } else if (D_801A2550.press & 0x8000) {
        D_80048630[i] = 0x8000;
        D_8017F4D8[i]->res = D_800BDE18[2];
    } else if (D_801A2550.press & 0x2000) {
        D_80048630[i] = 0x2000;
        D_8017F4D8[i]->res = D_800BDE18[3];
    } else if (D_801A2550.press & 0x4) {
        D_80048630[i] = 0x4;
        D_8017F4D8[i]->res = D_800BDE18[4];
    } else if (D_801A2550.press & 0x8) {
        D_80048630[i] = 0x8;
        D_8017F4D8[i]->res = D_800BDE18[5];
    } else if (D_801A2550.press & 0x1) {
        D_80048630[i] = 0x1;
        D_8017F4D8[i]->res = D_800BDE18[6];
    } else if (D_801A2550.press & 0x2) {
        D_80048630[i] = 0x2;
        D_8017F4D8[i]->res = D_800BDE18[7];
    } else if (D_801A2550.press & 0x40) {
        D_80048630[i] = 0x40;
        D_8017F4D8[i]->res = D_800BDE18[9];
    } else if (D_801A2550.press & 0x10) {
        D_80048630[i] = 0x10;
        D_8017F4D8[i]->res = D_800BDE18[10];
    } else if (D_801A2550.press & 0x20) {
        D_80048630[i] = 0x20;
        D_8017F4D8[i]->res = D_800BDE18[8];
    } else if (D_801A2550.press & 0x80) {
        D_80048630[i] = 0x80;
        D_8017F4D8[i]->res = D_800BDE18[11];
    } else {
        return;
    }

    SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
    D_801A2550.used = D_801A2550.used | D_801A2550.press | 0xF000;
    if (D_801A2574 != 0) {
        D_801A2550.delay1 = 6 / D_801A2574;
        if (!(D_801A2550.held & 0xF000)) {
            D_801A2550.delay2 = 12 / D_801A2574;
        }
    } else {
        D_801A2550.delay1 = 6;
        if (!(D_801A2550.held & 0xF000)) {
            D_801A2550.delay2 = 12;
        }
    }
    j = 0;
    do {
        D_8017F4D8[j]->light = 21;
        D_8017F508[j]->light = 21;
        j++;
    } while (j < 12);
    j = 0;
    do {
        for (k = 0; k < 12; k++) {
            if (D_80048630[j] == D_80048630[k] && j != k) {
                D_8017F4D8[k]->light = 13;
                D_8017F508[k]->light = 13;
            }
        }
        j++;
    } while (j < 12);
    o->update = func_800B0CF0;
}
