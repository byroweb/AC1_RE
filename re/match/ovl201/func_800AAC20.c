#include "ac1/ac1.h"

typedef struct { u8 r, g, b; } Rgb3;
typedef struct { u8 b[160]; } Blk160;

extern void func_800AAF20(void);
extern Blk160 D_80031A94;
extern Blk160 D_80040E60[];
extern Rgb3 D_80031A64[4][4];
extern Rgb3 D_8004AC60[][4][4];
extern u8 D_80031A58[];
extern u8 D_80040830[][12];
extern u16 D_80039CA0;
extern u16 D_80039D1C[];
extern void *D_801F36D0[];
extern s32 D_801BC478[];
extern void *D_80041A10[][30];
extern u16 D_80041858[][30];
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern u8 D_80039D28[];
extern u8 D_80039D12[];

void func_800AAC20(u8 which, UiObj *o)
{
    u8 *w = o->w.raw;
    s16 i;
    s16 j;

    D_80031A94 = D_80040E60[which];
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            D_80031A64[i][j] = D_8004AC60[which][i][j];
        }
    }
    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_80040830[which][i];
    }
    D_80039CA0 = D_80039D1C[which];
    for (i = 3; i < 15; i++) {
        D_801F36D0[i] = D_80041A10[which][i];
        D_801BC478[i] = D_80041858[which][i];
    }
    D_80039CA2 = D_80039D28[which];
    D_80039CA3 = D_80039D12[which];
    if ((which == 0 && w[5] == 1) || (which == 1 && w[5] == 0)) {
        func_800AAF20();
        if (w[5] == 0) {
            w[5] = 1;
        } else {
            w[5] = 0;
        }
    }
}
