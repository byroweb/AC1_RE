#include "ac1/ac1.h"

typedef struct {
    u8 col;             /* +0x7C */
    u8 row;             /* +0x7D */
} PadCfgWork;

extern u32 D_80048630[12];      /* config: pad mapping (button bit per action) */
extern char D_800BDE18[12][12]; /* button names */
extern char D_8004D8EC[];
extern char D_8004D8F8[];
extern char D_8004D900[];
extern char D_8004D90C[];
extern char D_8004D918[];
extern char D_8004D924[];
extern char D_8004D930[];
extern char D_8004D938[];
extern char D_8004D944[];
extern char D_8004D954[];
extern char D_8004D95C[];
extern char D_8004D96C[];
extern char D_8004D978[];
extern char D_8004D980[];
extern UiObj *D_8017F4D8[12];   /* button name labels */
extern UiObj *D_8017F508[12];   /* action labels */
extern void func_8005BA2C(void);
extern UiObj *func_8005D754(s32 kind, char *s, s32 x, s32 y, s32 n);
extern UiObj *func_8005D80C(s32 kind, char *s, s32 a, s32 x, s32 y, s32 n);
extern void func_800B0CF0(UiObj *o);
extern void func_800B1980(u8 *col, u8 *row);

/* Pad config screen setup (waits for the window): action and button labels, clash highlight. */
void func_800B0598(UiObj *o)
{
    u8 idx[12];
    s16 i;
    s16 j;

    if (obj_find_by_handle(func_8005BA2C) != &g_ui_none)
        return;
    i = 0;
    do {
        switch (D_80048630[i]) {
        case 0x1000:
            idx[i] = 0;
            break;
        case 0x4000:
            idx[i] = 1;
            break;
        case 0x8000:
            idx[i] = 2;
            break;
        case 0x2000:
            idx[i] = 3;
            break;
        case 0x4:
            idx[i] = 4;
            break;
        case 0x8:
            idx[i] = 5;
            break;
        case 0x1:
            idx[i] = 6;
            break;
        case 0x2:
            idx[i] = 7;
            break;
        case 0x20:
            idx[i] = 8;
            break;
        case 0x40:
            idx[i] = 9;
            break;
        case 0x10:
            idx[i] = 10;
            break;
        case 0x80:
            idx[i] = 11;
            break;
        }
        i++;
    } while (i < 12);
    D_8017F4D8[0] = func_8005D754(3, D_800BDE18[idx[0]], 126, 16, 8);
    D_8017F4D8[1] = func_8005D754(3, D_800BDE18[idx[1]], 126, 32, 8);
    D_8017F4D8[2] = func_8005D754(3, D_800BDE18[idx[2]], 126, 48, 8);
    D_8017F4D8[3] = func_8005D754(3, D_800BDE18[idx[3]], 126, 64, 8);
    D_8017F4D8[4] = func_8005D754(3, D_800BDE18[idx[4]], 126, 80, 8);
    D_8017F4D8[5] = func_8005D754(3, D_800BDE18[idx[5]], 126, 96, 8);
    D_8017F4D8[6] = func_8005D754(3, D_800BDE18[idx[6]], 126, 112, 8);
    D_8017F4D8[7] = func_8005D754(3, D_800BDE18[idx[7]], 126, 128, 8);
    D_8017F4D8[8] = func_8005D754(3, D_800BDE18[idx[8]], 272, 16, 8);
    D_8017F4D8[9] = func_8005D754(3, D_800BDE18[idx[9]], 272, 32, 8);
    D_8017F4D8[10] = func_8005D754(3, D_800BDE18[idx[10]], 272, 48, 8);
    D_8017F4D8[11] = func_8005D754(3, D_800BDE18[idx[11]], 272, 64, 8);
    D_8017F508[0] = func_8005D80C(3, D_8004D8EC, 16, 8, 16, 8);
    D_8017F508[1] = func_8005D80C(3, D_8004D8F8, 16, 8, 32, 8);
    D_8017F508[2] = func_8005D80C(3, D_8004D900, 16, 8, 48, 8);
    D_8017F508[3] = func_8005D80C(3, D_8004D90C, 16, 8, 64, 8);
    D_8017F508[4] = func_8005D80C(3, D_8004D918, 16, 8, 80, 8);
    D_8017F508[5] = func_8005D80C(3, D_8004D924, 16, 8, 96, 8);
    D_8017F508[6] = func_8005D80C(3, D_8004D930, 16, 8, 112, 8);
    D_8017F508[7] = func_8005D80C(3, D_8004D938, 16, 8, 128, 8);
    D_8017F508[8] = func_8005D80C(3, D_8004D944, 16, 152, 16, 8);
    D_8017F508[9] = func_8005D80C(3, D_8004D954, 16, 152, 32, 8);
    D_8017F508[10] = func_8005D80C(3, D_8004D95C, 16, 152, 48, 8);
    D_8017F508[11] = func_8005D80C(3, D_8004D96C, 16, 152, 64, 8);
    func_8005D80C(3, D_8004D978, 16, 152, 112, 8);
    func_8005D80C(3, D_8004D980, 16, 152, 128, 8);
    i = 0;
    do {
        for (j = 0; j < 12; j++) {
            if (D_80048630[i] == D_80048630[j] && i != j) {
                D_8017F4D8[j]->light = 13;
                D_8017F508[j]->light = 13;
            }
        }
        i++;
    } while (i < 12);
    func_800B1980(&((PadCfgWork *)&o->w)->col, &((PadCfgWork *)&o->w)->row);
    o->update = func_800B0CF0;
}
