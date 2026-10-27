/* ovl201 0x8007AD64: matched, no asm, no extra flags. arm is an s32 that takes (u8)D_80031A5E inside the || test
 * (so the compare temp is arm itself: andi v1,a0) and D_80031A5F in the then-arm; `grey = 21 + (arm >> 8);` keeps
 * the then-read live at flow time (combine folds arm >> 8 to 0, arm's sets are byte loads), so arm is one global
 * pseudo in v1 and the read needs no volatile. */
#include "ac1/ac1.h"
/* ovl201 0x8007AD64 - assembly screen: once the func_8005BA2C object is gone,
 * set up the two 128 x 144 draw environments, show the warning icons
 * (part missing, credits below zero, arm or leg overload) and the names of
 * the equipped parts ("NO EQUIP" for an empty slot). */

typedef struct {
    RECT clip;
    s16 ofs[2];
    u8 rest[0x50];
} DRAWENV;

typedef struct { char name[0x28]; u16 weight; u8 pad[0x12]; } PartHead;     /* 60 */
typedef struct { char name[0x28]; u16 weight; u8 pad[8]; u16 maxload; u8 pad2[4]; } PartCore; /* 56 */
typedef struct { char name[0x28]; u16 weight; u8 pad[0x0A]; } PartArms;     /* 52 */
typedef struct { char name[0x28]; u16 weight; u8 pad[8]; u16 maxload; u8 pad2[0x14]; } PartLegs; /* 72 */
typedef struct { char name[0x28]; u16 weight; u8 pad[6]; } Part48;          /* 48 */
typedef struct { char name[0x28]; u16 weight; u8 pad[0x36]; } Part96;       /* 96 */
typedef struct { char name[0x28]; u16 weight; u8 pad[2]; } Part44;          /* 44 */

extern DRAWENV D_801BD018;
extern DRAWENV D_801D6470;
extern DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);

extern u8 D_80031A58;   /* core */
extern u8 D_80031A59;   /* booster */
extern u8 D_80031A5A;   /* head */
extern u8 D_80031A5B;   /* arms */
extern u8 D_80031A5C;
extern u8 D_80031A5D;
extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern u8 D_80031A60;
extern u8 D_80031A61;   /* FCS */
extern u8 D_80031A63;   /* legs */
extern s32 D_8004AC38;
extern char D_8004C48C[];
extern u8 D_800B840C[];
extern u8 D_800B8418[];
extern u8 D_800B8424[];
extern u8 D_800B8430[];
extern PartHead D_800B91F4[];
extern PartCore D_800B944C[];
extern PartLegs D_800B94F4[];
extern PartArms D_800B9DF4[];
extern Part44 D_800BA134[];
extern PartArms D_800BA3F4[];
extern PartArms D_800BAADC[];
extern Part48 D_800BABAC[];
extern Part96 D_800BACCC[];
extern Part48 D_800BAFCC[];

extern void func_8005BA2C(void);
extern void func_8005CAE0(u8 *spr, s32 x, s32 y, s32 z);
extern UiObj *func_8005D80C(s32 a, char *s, s32 b, s32 c, s32 d, s32 e);
extern void func_800740AC(UiObj *o);
extern void func_8007B850(UiObj *o);

void func_8007AD64(UiObj *o)
{
    s32 arm;
    s16 grey;
    u16 w;
    u16 total;

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        SetDefDrawEnv(&D_801BD018, 176, 24, 128, 144);
        SetDefDrawEnv(&D_801D6470, 176, 264, 128, 144);
        D_801BD018.ofs[0] = D_801BD018.clip.x + (D_801BD018.clip.w - 320) / 2;
        D_801BD018.ofs[1] = D_801BD018.clip.y + (D_801BD018.clip.h - 240) / 2;
        D_801D6470.ofs[0] = D_801D6470.clip.x + (D_801D6470.clip.w - 320) / 2;
        D_801D6470.ofs[1] = D_801D6470.clip.y + (D_801D6470.clip.h - 240) / 2;
        if ((D_80031A5E >= 24 && D_80031A5E < 31) || (arm = D_80031A5E) == 255) {
            arm = D_80031A5F;
        }
        grey = 21 + (arm >> 8);
        if (D_80031A5A == 255 || D_80031A58 == 255 || D_80031A5B == 255 || D_80031A60 == 255
            || D_80031A61 == 255 || D_80031A63 == 255 || (D_80031A63 < 27 && D_80031A59 == 255)) {
            func_8005CAE0(D_800B840C, 20, 8, 10);
        }
        *(u16 *)&o->w.raw[0x2A] = (&D_80031A58)[*(u16 *)&o->w.raw[0x16]];
        func_800740AC(o);
        if (D_8004AC38 < 0) {
            func_8005CAE0(D_800B8418, 20, 40, 10);
        }
        if (D_80031A5B != 255) {
            w = D_800B9DF4[D_80031A5B].weight;
            if (D_80031A5B < 8) {
                if (D_80031A5D != 255) {
                    w += D_800BA134[D_80031A5D].weight;
                }
                if (D_80031A5C != 255) {
                    w += D_800BAADC[D_80031A5C].weight;
                }
            }
            if (D_800B944C[D_80031A58].maxload < w) {
                func_8005CAE0(D_800B8424, 20, 72, 10);
            }
        }
        total = D_800B944C[D_80031A58].weight;
        if (D_80031A5B != 255) {
            total += D_800B9DF4[D_80031A5B].weight;
        }
        if (D_80031A5A != 255) {
            total += D_800B91F4[D_80031A5A].weight;
        }
        if (D_80031A60 != 255) {
            total += D_800BAFCC[D_80031A60].weight;
        }
        if (D_80031A61 != 255) {
            total += D_800BACCC[D_80031A61].weight;
        }
        if (D_80031A59 != 255) {
            total += D_800BABAC[D_80031A59].weight;
        }
        if (D_80031A5E != 255) {
            total += D_800BA3F4[D_80031A5E].weight;
        }
        if (D_80031A5F != 255) {
            total += D_800BA3F4[D_80031A5F].weight;
        }
        if (D_80031A5C != 255) {
            total += D_800BAADC[D_80031A5C].weight;
        }
        if (D_80031A5D != 255) {
            total += D_800BA134[D_80031A5D].weight;
        }
        if (D_80031A63 == 255 || D_800B94F4[D_80031A63].maxload < total) {
            func_8005CAE0(D_800B8430, 20, 104, 10);
        }
        if (D_80031A5A == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 10, 4)->light = 15;
        } else {
            func_8005D80C(1, D_800B91F4[D_80031A5A].name, 6, 46, 10, 4);
        }
        if (D_80031A58 == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 20, 4)->light = 15;
        } else {
            func_8005D80C(1, D_800B944C[D_80031A58].name, 6, 46, 20, 4);
        }
        if (D_80031A5B == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 30, 4)->light = 15;
        } else {
            func_8005D80C(1, D_800B9DF4[D_80031A5B].name, 6, 46, 30, 4);
        }
        if (D_80031A63 == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 40, 4)->light = 15;
        } else {
            func_8005D80C(1, D_800B94F4[D_80031A63].name, 6, 46, 40, 4);
        }
        if (D_80031A60 == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 100, 4)->light = 15;
        } else {
            func_8005D80C(1, D_800BAFCC[D_80031A60].name, 6, 46, 100, 4);
        }
        if (D_80031A61 == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 110, 4)->light = 15;
        } else {
            func_8005D80C(1, D_800BACCC[D_80031A61].name, 6, 46, 110, 4);
        }
        if (D_80031A59 == 255) {
            if (D_80031A63 < 27) {
                func_8005D80C(1, D_8004C48C, 6, 46, 90, 4)->light = 15;
            } else {
                func_8005D80C(1, D_8004C48C, 6, 46, 90, 4);
            }
        } else {
            func_8005D80C(1, D_800BABAC[D_80031A59].name, 6, 46, 90, 4);
        }
        if (D_80031A5E == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 80, 4)->light = grey;
        } else {
            func_8005D80C(1, D_800BA3F4[D_80031A5E].name, 6, 46, 80, 4);
        }
        if (D_80031A5F == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 70, 4)->light = grey;
        } else {
            func_8005D80C(1, D_800BA3F4[D_80031A5F].name, 6, 46, 70, 4);
        }
        if (D_80031A5C == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 60, 4);
        } else {
            func_8005D80C(1, D_800BAADC[D_80031A5C].name, 6, 46, 60, 4);
        }
        if (D_80031A5D == 255) {
            func_8005D80C(1, D_8004C48C, 6, 46, 50, 4)->light = grey;
        } else {
            func_8005D80C(1, D_800BA134[D_80031A5D].name, 6, 46, 50, 4);
        }
        o->update = func_8007B850;
    }
}
