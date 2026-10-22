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

extern u8 D_80031A58[];
extern u8 D_8017EE20[];
void func_80071A18(void);

void func_800740AC(UiObj *o)
{
    AsmWork *w = &((AsmObj *)o)->w;
    s16 i;

    for (i = 0; i < 12; i++)
        D_8017EE20[i] = D_80031A58[i];
    D_80031A58[w->h92] = w->hA6;
    if (D_80031A58[6] != 255 && D_80031A58[6] >= 31)
        D_80031A58[7] = 255;
    if (D_80031A58[7] != 255 && D_80031A58[7] >= 31)
        D_80031A58[6] = 255;
    if (D_80031A58[3] >= 8 || D_80031A58[3] == 255) {
        D_80031A58[5] = 255;
        D_80031A58[4] = 255;
    }
    if (D_80031A58[11] >= 27 && D_80031A58[11] != 255) {
        D_80031A58[1] = 255;
        if (D_80031A58[6] != 255 && D_80031A58[6] >= 24 && D_80031A58[6] <= 30
            && D_80031A58[6] != 27 && D_80031A58[6] != 29)
            D_80031A58[6] = 255;
        if (D_80031A58[7] != 255 && D_80031A58[7] >= 24 && D_80031A58[7] <= 30
            && D_80031A58[7] != 27 && D_80031A58[7] != 29)
            D_80031A58[7] = 255;
    }
    if (D_80031A58[5] == 15) {
        switch (D_80031A58[11]) {
        case 5:
        case 21:
        case 22:
        case 23:
        case 26:
        case 27:
        case 28:
        case 30:
        case 31:
            D_80031A58[5] = 255;
            break;
        }
    }
    if (D_80031A58[3] == 5) {
        if (D_80031A58[6] != 255 && (D_80031A58[6] < 24 || D_80031A58[6] == 32))
            D_80031A58[6] = 255;
        if (D_80031A58[7] != 255 && (D_80031A58[7] < 24 || D_80031A58[7] == 32))
            D_80031A58[7] = 255;
        if (D_80031A58[6] == 32 || D_80031A58[7] == 32) {
            D_80031A58[7] = 255;
            D_80031A58[6] = 255;
        }
    }
    func_80071A18();
    for (i = 0; i < 12; i++)
        D_80031A58[i] = D_8017EE20[i];
}
