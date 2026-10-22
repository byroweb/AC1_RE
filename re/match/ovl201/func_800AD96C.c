#include "ac1/ac1.h"

typedef struct { u8 r, g, b; } Rgb3;

extern void func_800AAF20(void);
extern void func_80051078(void *);
extern void *D_801F36D0[];
extern s32 D_801BC478[];
extern Rgb3 D_80031A64[4][4];
extern Rgb3 D_8004AC60[4][4];
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern u8 D_80039D28;
extern u8 D_80039D12;
extern void *D_80041A10[2][30];
extern s16 D_80041858[2][30];

void func_800AD96C(UiObj *o)
{
    s16 i;
    s16 j;
    s16 k;

    if (o->w.raw[5] == 1) {
        func_800AAF20();
    }
    for (i = 3; i < 15; i++) {
        D_801F36D0[i] = 0;
        D_801BC478[i] = 0;
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                D_80031A64[j][k] = D_8004AC60[j][k];
            }
        }
        D_80039CA2 = D_80039D28;
        D_80039CA3 = D_80039D12;
        if (D_80041A10[0][i] != 0) {
            func_80051078(D_80041A10[0][i]);
            D_80041A10[0][i] = 0;
            D_80041858[0][i] = 0;
        }
        if (D_80041A10[1][i] != 0) {
            func_80051078(D_80041A10[1][i]);
            D_80041A10[1][i] = 0;
            D_80041858[1][i] = 0;
        }
    }
}
