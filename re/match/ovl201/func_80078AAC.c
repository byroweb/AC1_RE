#include "ac1/ac1.h"
/* local view of the assembly-screen object's work area (UiObj +0x7C) */
typedef struct AsmWork {
    void *src;          /* +0x7C */
    UiObj *owner;       /* +0x80 */
    u8 b84, b85;        /* +0x84 */
    u8 b86, b87;        /* +0x86 */
    UiObj *sub;         /* +0x88 */
    u8 pad8c[4];
    u16 h90;            /* +0x90 part category */
    u16 h92;            /* +0x92 selector mode */
    s16 h94;            /* +0x94 */
    u8 b96;             /* +0x96 */
    u8 b97;             /* +0x97 */
    u8 b98, b99, b9a;   /* +0x98 */
    u8 pad9b;
    UiObj *bar;         /* +0x9C */
    u8 bA0, bA1;        /* +0xA0 */
    s16 hA2, hA4;       /* +0xA2 */
    u16 hA6;            /* +0xA6 part index in the category, 255 = none */
    s16 hA8;            /* +0xA8 */
    u8 padAA;
    u8 bAB;             /* +0xAB */
    s16 hAC;            /* +0xAC */
    u8 bAE;             /* +0xAE */
    u8 padAF;
    u16 hB0;            /* +0xB0 */
} AsmWork;

typedef struct AsmObj {
    u8 head[0x7C];
    AsmWork w;
} AsmObj;

extern u8 D_80031A5B;
extern u8 D_80031A63;
extern char D_800B8448[];
extern UiObj *D_8017EF80[7];
extern s32 D_801BC478[];
extern s32 D_801F36D0[];
void func_80051078(s32);
UiObj *func_8005CAE0(void *, s32, s32, s32);
void func_800740AC(UiObj *);
void func_8007509C(UiObj *);
void func_80075520(void);
void func_80075898(UiObj *);
void func_80078D84(UiObj *);
UiObj *func_8007BE0C(s32, UiObj *, s32);

void func_80078AAC(UiObj *o)
{
    AsmObj *a = (AsmObj *)o;
    AsmWork *w;
    s16 n;

    n = 1;
    w = &a->w;
    if (a->w.bA1 == 1)
        n = 2;
    if (D_801F36D0[n] != 0) {
        func_80051078(D_801F36D0[n]);
        D_801F36D0[n] = 0;
        D_801BC478[n] = 0;
    }
    a->w.bar = func_8007BE0C(a->w.bA1, o, a->w.bA0);
    a->w.bar->rot.vy = a->w.hA2;
    a->w.bar->w.panel.rank = a->w.hA4;
    func_8007509C(o);
    func_80075898(o);
    func_800740AC(o);
    func_80075520();
    D_8017EF80[6]->kill = 1;
    D_8017EF80[6] = &g_ui_none;
    switch (a->w.h92) {
    case 4:
    case 5:
        if (D_80031A5B == 255 || D_80031A5B >= 8) {
            if (w->hA6 != 255)
                D_8017EF80[6] = func_8005CAE0(D_800B8448, 20, 48, 4);
        } else {
            if (w->hA6 == 15) {
                switch (D_80031A63) {
                case 5:
                case 21:
                case 22:
                case 23:
                case 26:
                case 27:
                case 28:
                case 30:
                case 31:
                    D_8017EF80[6] = func_8005CAE0(D_800B8448, 20, 48, 4);
                    break;
                }
            }
        }
        break;
    case 6:
    case 7:
        if (D_80031A63 != 255 && D_80031A63 >= 27) {
            switch (w->hA6) {
            case 24:
            case 25:
            case 26:
            case 28:
            case 30:
                D_8017EF80[6] = func_8005CAE0(D_800B8448, 20, 48, 4);
                break;
            }
        }
        if (D_80031A5B == 5 && ((w->hA6 != 255 && w->hA6 < 24) || w->hA6 == 32)) {
            if (D_8017EF80[6] != &g_ui_none)
                D_8017EF80[6]->kill = 1;
            D_8017EF80[6] = func_8005CAE0(D_800B8448, 20, 48, 4);
        }
        break;
    case 1:
        if (D_80031A63 != 255 && D_80031A63 >= 27 && w->hA6 != 255)
            D_8017EF80[6] = func_8005CAE0(D_800B8448, 20, 48, 4);
        break;
    }
    o->update = func_80078D84;
}
