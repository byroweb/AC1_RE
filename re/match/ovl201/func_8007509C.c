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

/* part tables: name first, then three stats; stride differs per category */
#define PART(n, sz) typedef struct { char name[0x28]; u16 s28; u16 s2a; u16 s2c; u8 pad[sz - 0x2E]; } n
PART(Part3C, 0x3C);
PART(Part38, 0x38);
PART(Part48, 0x48);
PART(Part34, 0x34);
typedef struct { char name[0x28]; u16 s28; u16 s2a; } Part2C;
PART(Part30, 0x30);
PART(Part60, 0x60);

extern Part3C D_800B91F4[];
extern Part38 D_800B944C[];
extern Part48 D_800B94F4[];
extern Part34 D_800B9DF4[];
extern Part2C D_800BA134[];
extern Part34 D_800BA3F4[];
extern Part34 D_800BAADC[];
extern Part30 D_800BABAC[];
extern Part60 D_800BACCC[];
extern Part30 D_800BAFCC[];
extern char D_800BB870[];
extern char D_8017EE2C[4][7];
extern UiObj *D_8017EF80;
extern UiObj *D_8017EF84;
extern UiObj *D_8017EF88;
extern UiObj *D_8017EF8C;
extern UiObj *D_8017EF90;
extern UiObj *D_8017EF94;
UiObj *func_8005D80C(s32 layer, char *s, s32 x, s32 w, s32 y, s32 kind);
void func_80074D40(char *buf, s16 n);
u8 func_80083C00(u16 cat, u16 idx);

void func_8007509C(UiObj *o)
{
    AsmWork *w;
    char *name;
    s16 a, b, c, d;

    w = &((AsmObj *)o)->w;
    if (w->hA6 == 255) {
        name = D_800BB870;
        d = -1;
        c = -1;
        b = -1;
        a = -1;
    } else {
        switch (w->h90) {
        case 4:
            c = D_800B91F4[w->hA6].s28;
            b = D_800B91F4[w->hA6].s2c;
            a = D_800B91F4[w->hA6].s2a;
            name = D_800B91F4[w->hA6].name;
            break;
        case 0:
            c = D_800B944C[w->hA6].s28;
            b = D_800B944C[w->hA6].s2c;
            a = D_800B944C[w->hA6].s2a;
            name = D_800B944C[w->hA6].name;
            break;
        case 1:
            c = D_800B94F4[w->hA6].s28;
            b = D_800B94F4[w->hA6].s2c;
            a = D_800B94F4[w->hA6].s2a;
            name = D_800B94F4[w->hA6].name;
            break;
        case 2:
            c = D_800B9DF4[w->hA6].s28;
            b = D_800B9DF4[w->hA6].s2c;
            a = D_800B9DF4[w->hA6].s2a;
            name = D_800B9DF4[w->hA6].name;
            break;
        case 7:
            b = 0;
            c = D_800BA134[w->hA6].s28;
            a = D_800BA134[w->hA6].s2a;
            name = D_800BA134[w->hA6].name;
            break;
        case 6:
            b = 0;
            c = D_800BA3F4[w->hA6].s28;
            a = D_800BA3F4[w->hA6].s2a;
            name = D_800BA3F4[w->hA6].name;
            break;
        case 3:
            b = 0;
            c = D_800BAADC[w->hA6].s28;
            a = D_800BAADC[w->hA6].s2a;
            name = D_800BAADC[w->hA6].name;
            break;
        case 5:
            b = 0;
            c = D_800BABAC[w->hA6].s28;
            a = D_800BABAC[w->hA6].s2a;
            name = D_800BABAC[w->hA6].name;
            break;
        case 8:
            b = 0;
            c = D_800BACCC[w->hA6].s28;
            a = D_800BACCC[w->hA6].s2a;
            name = D_800BACCC[w->hA6].name;
            break;
        case 10:
            b = 0;
            a = 0;
            name = D_800BAFCC[w->hA6].name;
            c = D_800BAFCC[w->hA6].s28;
            break;
        }
        d = func_80083C00(w->h90, w->hA6);
    }
    D_8017EF80->kill = 1;
    D_8017EF84->kill = 1;
    D_8017EF88->kill = 1;
    D_8017EF8C->kill = 1;
    D_8017EF90->kill = 1;
    D_8017EF94->kill = 1;
    D_8017EF80 = func_8005D80C(1, name, 6, 0x2E, 0xAC, 4);
    D_8017EF84 = &g_ui_none;
    func_80074D40(D_8017EE2C[0], a);
    func_80074D40(D_8017EE2C[1], b);
    func_80074D40(D_8017EE2C[2], c);
    func_80074D40(D_8017EE2C[3], d);
    D_8017EF88 = func_8005D80C(1, D_8017EE2C[0], 6, 0x44, 0x84, 4);
    D_8017EF8C = &g_ui_none;
    D_8017EF90 = func_8005D80C(1, D_8017EE2C[2], 6, 0x44, 0x8C, 4);
    D_8017EF94 = &g_ui_none;
}
