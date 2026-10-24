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
    u8 col;             /* +0x7C 0 actions, 1 second column */
    u8 row;             /* +0x7D */
} PadCfgWork;

typedef struct {
    s32 key[12];
} PadMap;

extern PadState D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern PadMap D_80048630;       /* config: pad mapping */
extern PadMap D_800BDDA0;       /* default pad mapping */
extern char D_800BDE18[12][12]; /* button names */
extern char D_8004D988[];
extern UiObj *D_8017F4D8[12];
extern UiObj *D_8017F508[12];
extern void func_800506EC(s32 a);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 n);
extern void func_8005DED8(UiObj *o, void (*back)(UiObj *), char *msg, s32 n);
extern void func_800B146C(UiObj *o);
extern void func_800B1924(UiObj *o);

/* Pad config screen: move the cursor, confirm picks an action (then a button),
 * "default" resets the map, exit/cancel refuse while two actions share a button. */
void func_800B0CF0(UiObj *o)
{
    PadCfgWork *w = (PadCfgWork *)&o->w;
    s16 i;
    s16 j;
    s16 k;
    s16 m;
    u8 r;

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
        if (w->col == 0) {
            if (w->row == 0) {
                w->row = 7;
            } else {
                w->row--;
            }
        } else {
            switch (w->row) {
            case 0:
                w->row = 7;
                break;
            case 6:
                w->row = 3;
                break;
            default:
                w->row--;
                break;
            }
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
        if (w->col == 0) {
            r = w->row;
            if (r == 7) {
                w->row = 0;
            } else {
                w->row = r + 1;
            }
        } else {
            r = w->row;
            switch (r) {
            case 7:
                w->row = 0;
                break;
            case 3:
                w->row = 6;
                break;
            default:
                w->row = r + 1;
                break;
            }
        }
    } else if (D_801A2550.press & 0x2000) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        D_801A2550.used |= 0x2000;
        if (D_801A2574 != 0) {
            D_801A2550.delay1 = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.delay2 = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay1 = 6;
            if (!(D_801A2550.held & 0x2000)) {
                D_801A2550.delay2 = 12;
            }
        }
        if (w->col == 1) {
            w->col = 0;
        } else if ((s16)w->row >= 6) {
            w->col = 1;
        } else if ((s16)w->row < 4) {
            w->col = 1;
        }
    } else if (D_801A2550.press & 0x8000) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        D_801A2550.used |= 0x8000;
        if (D_801A2574 != 0) {
            D_801A2550.delay1 = 6 / D_801A2574;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.delay2 = 12 / D_801A2574;
            }
        } else {
            D_801A2550.delay1 = 6;
            if (!(D_801A2550.held & 0x8000)) {
                D_801A2550.delay2 = 12;
            }
        }
        if (w->col == 0) {
            if ((s16)w->row >= 6) {
                w->col = 1;
            } else if ((s16)w->row < 4) {
                w->col = 1;
            }
        } else {
            w->col = 0;
        }
    } else if (D_801A2550.press & 0x40) {
        D_801A2550.used |= 0x40;
        SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        if (*(u16 *)w == 0x601) {
            D_80048630 = D_800BDDA0;
            i = 0;
            do {
                D_8017F4D8[i]->res = D_800BDE18[i];
                D_8017F4D8[i]->light = 21;
                D_8017F508[i]->light = 21;
                i++;
            } while (i < 12);
        } else if (*(u16 *)w == 0x701) {
            k = 0;
            do {
                j = 0;
                do {
                    if (D_80048630.key[k] == D_80048630.key[j] && k != j) {
                        func_8005DED8(o, func_800B0CF0, D_8004D988, 10);
                        o->update = ui_obj_nop2;
                        return;
                    }
                    j++;
                } while (j < 12);
                k++;
            } while (k < 12);
            func_800506EC(8);
            obj_find_by_id(7)->kill = 1;
            func_8005C2D0(12, 48, 296, 160, 7);
            o->update = func_800B1924;
        } else {
            D_8017F4D8[w->col * 8 + w->row]->light = 11;
            D_8017F508[w->col * 8 + w->row]->light = 11;
            o->update = func_800B146C;
        }
    } else if (D_801A2550.press & 0x20) {
        D_801A2550.used |= 0x20;
        SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        m = 0;
        do {
            j = 0;
            do {
                if (D_80048630.key[m] == D_80048630.key[j] && m != j) {
                    func_8005DED8(o, func_800B0CF0, D_8004D988, 10);
                    o->update = ui_obj_nop2;
                    return;
                }
                j++;
            } while (j < 12);
            m++;
        } while (m < 12);
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_8005C2D0(12, 48, 296, 160, 7);
        o->update = func_800B1924;
    }
}
