#include "ac1/ac1.h"

typedef struct {
    u8 col[3];                  /* +0x00 screen fade colour */
    u8 fade;                    /* +0x03 0 = off */
    u8 pad04[0xDD8];
    MATRIX cam;                 /* +0xDDC = 0x801AD67C */
} ScreenFx;

typedef struct {                /* 0x3C */
    u8 kind;                    /* +0x00 0, 1, 2 */
    u8 pad01[0xB];
    u8 flags;                   /* +0x0C */
    u8 pad0d[0x2F];
} HudMode;

typedef struct {
    TargetSlot slot[8];         /* +0x000 = g_target_list */
    u8 pad180[0xD4];
    u8 on;                      /* +0x254 = 0x80041278 */
    u8 pad255;
    HudMode mode[1];            /* +0x256 */
} TgtWork;

extern ScreenFx D_801AC8A0;
extern TgtWork D_80041024;
extern s32 D_801AC80C;
extern u8 D_80040FB8;
extern u8 D_8004121A;
extern u8 D_801A26F4;
extern u8 D_801A26F5;
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void func_8005E7D8(s32 x0, s32 y0, s32 x1, s32 y1, u8 *col, s32 z, s32 abr);
extern void func_80051148(void);
extern void func_80050468(void);
extern void func_8005156C(HudMode *m, TargetSlot *t);
extern void func_800519F8(TargetSlot *t);
extern void func_80051D04(TargetSlot *t);
extern void func_80051F28(TargetSlot *t);
extern void func_80052048(TargetSlot *t);

void func_8006260C(void)
{
    HudMode *m;
    TargetSlot *t;
    s32 i;

    if (D_801AC8A0.fade) {
        func_8005E7D8(0, 0, 320, 240, D_801AC8A0.col, 1, D_801AC8A0.fade);
    }
    func_80051148();
    if (D_801AC80C == 0) {
        return;
    }
    func_80050468();
    if (D_80041024.on == 0 || D_801A26F4 == 0x20) {
        return;
    }
    SetRotMatrix(&D_801AC8A0.cam);
    SetTransMatrix((MATRIX *)((u8 *)&D_801AC8A0.cam + 12));
    m = &D_80041024.mode[D_80040FB8];
    switch (m->kind) {
    case 0:
        if (!(m->flags & 4)) {
            func_800519F8(D_80041024.slot);
        }
        func_8005156C(m, g_target_list);
        break;
    case 1:
        i = 8;
        t = D_80041024.slot;
        do {
            func_80051D04(t);
            t++;
        } while (--i);
        func_80051F28(g_target_head);
        func_8005156C(m, 0);
        break;
    case 2:
        if (D_801A26F4 == 0x22 || D_8004121A == 1 || D_8004121A == 3
            || (D_8004121A == 2 && D_801A26F5 != 0xC0)) {
            func_80052048(g_target_list);
            func_8005156C(m, g_target_list);
        }
        break;
    }
}
