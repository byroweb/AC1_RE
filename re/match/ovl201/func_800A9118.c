#include "ac1/ac1.h"

typedef struct { u8 r, g, b; } Rgb3;

extern s16 func_800517C8(void);
extern void func_8005EB80(s32);
extern void func_80074A18(void);
extern void func_80073740(void);
extern void func_800A94F4(UiObj *);
extern u16 D_80039D08;
extern u16 D_80039D0A;
extern u16 D_80039D0C;
extern u16 D_80039D0E;
extern u8 D_80039CAB;
extern RECT D_8017F4C0;
extern void *D_801F36D0[];
extern s32 D_801BC478[];
extern void *D_80041A10[2][30];
extern s16 D_80041858[2][30];
extern u8 D_80031A58[];
extern u8 D_8004083C[];
extern Rgb3 D_80031A64[4][4];
extern Rgb3 D_8004AC90[4][4];
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern u8 D_80039D29;
extern u8 D_80039D13;

void func_800A9118(UiObj *o)
{
    s16 i;
    s16 j;
    s16 k;
    s16 m;

    if (func_800517C8() == 0) {
        u16 *p = &D_80039D08;
        if (*p >= 1000) {
            *p = 999;
        }
        if (D_80039D0A >= 1000) {
            D_80039D0A = 999;
        }
        if (D_80039D0C >= 1000) {
            D_80039D0C = 999;
        }
        if (D_80039D0E >= 1000) {
            D_80039D0E = 999;
        }
        if (D_80039CAB == 0) {
            D_8017F4C0.x = 704;
            D_8017F4C0.y = 256;
            D_8017F4C0.w = 128;
            D_8017F4C0.h = 256;
            MoveImage(&D_8017F4C0, 704, 0);
            D_8017F4C0.x = 544;
            D_8017F4C0.y = 384;
            D_8017F4C0.w = 16;
            D_8017F4C0.h = 64;
            MoveImage(&D_8017F4C0, 560, 384);
            D_8017F4C0.x = 0;
            D_8017F4C0.y = 486;
            D_8017F4C0.w = 256;
            D_8017F4C0.h = 9;
            MoveImage(&D_8017F4C0, 0, 495);
        }
        D_8017F4C0.x = 320;
        D_8017F4C0.y = 256;
        D_8017F4C0.w = 64;
        D_8017F4C0.h = 224;
        MoveImage(&D_8017F4C0, 448, 256);
        D_8017F4C0.x = 384;
        D_8017F4C0.y = 256;
        D_8017F4C0.w = 64;
        D_8017F4C0.h = 224;
        MoveImage(&D_8017F4C0, 832, 0);
        D_8017F4C0.x = 320;
        D_8017F4C0.y = 480;
        D_8017F4C0.w = 256;
        D_8017F4C0.h = 15;
        MoveImage(&D_8017F4C0, 320, 496);
        DrawSync(0);
        for (i = 3; i < 15; i++) {
            D_80041A10[0][i] = D_801F36D0[i];
            D_80041858[0][i] = D_801BC478[i];
            D_801F36D0[i] = 0;
            D_801BC478[i] = 0;
        }
        for (k = 0; k < 12; k++) {
            D_80031A58[k] = D_8004083C[k];
        }
        D_80039CA2 = D_80039D29;
        D_80039CA3 = D_80039D13;
        for (m = 0; m < 4; m++) {
            for (j = 0; j < 4; j++) {
                D_80031A64[m][j] = D_8004AC90[m][j];
            }
        }
        func_8005EB80(1);
        func_80074A18();
        func_80073740();
        o->update = func_800A94F4;
    }
}
