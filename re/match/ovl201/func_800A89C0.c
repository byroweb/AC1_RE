#include "ac1/ac1.h"

typedef struct { u8 r, g, b; } Rgb3;
typedef struct { u8 b[160]; } Blk160;
typedef struct { u8 b[36]; } Blk36;
typedef struct { s32 w[12]; } Blk48;
typedef struct { u8 b[7]; } Blk7;

extern s16 func_800517C8(void);
extern UiObj *func_8005D754(s32, void *, s32, s32, s32);
extern void func_8005AE8C();
extern void func_800AE7A4(void);
extern void func_8005B2CC(void);
extern void func_8005D39C(s32);
extern void func_80051450(s32, s32);
extern void func_800A8FFC(UiObj *);
extern u8 D_800BDD70[];
extern u8 D_80039CAB;
extern u8 D_80039CAE;
extern u8 D_80048663[];
extern u8 D_80039D14;
extern u16 D_80039CA0;
extern Blk48 D_80048630;
extern Blk48 D_80048538[];
extern Blk7 D_80040E38[];
extern Blk36 D_80031BD4;
extern Blk36 D_800419B8[];
extern Blk160 D_80031A94;
extern Blk160 D_80040E60[];
extern Rgb3 D_80031A64[4][4];
extern Rgb3 D_8004AC60[][4][4];
extern u8 D_80031A58[];
extern u8 D_80040830[][12];
extern u16 D_80039D1C[];
extern u16 D_80039D08[];
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern u8 D_80039D28[];
extern u8 D_80039D12[];

void func_800A89C0(UiObj *o)
{
    UiObj *p;
    u8 v;
    s32 r;
    s16 i;
    s16 j;

    if (func_800517C8() == 0) {
        p = func_8005D754(1, D_800BDD70, 38, 96, 2);
        o->w.ring.item[3] = p;
        p->light = 11;
        if (D_80039CAB == 0) {
            D_80039CAE = 80;
        }
        if (D_80048663[0] == 1) {
            r = D_80039D14;
            if (r >= 205) {
                v = 80;
            } else if (r >= 154) {
                v = 81;
            } else if (r >= 103) {
                v = 82;
            } else if (r >= 52) {
                v = 83;
            } else {
                v = 84;
            }
            D_80039CAE = v;
        } else if (D_80048663[0] == 2) {
            if (++D_80039CAE == 85) {
                D_80039CAE = 80;
            }
        }
        if (D_80039CAB == 0) {
            func_8005AE8C();
            D_80039CA0 = 0;
            func_800AE7A4();
            func_8005B2CC();
            D_80048538[0] = D_80048538[1] = D_80048630;
            D_80040E38[0].b[0] = D_80040E38[1].b[0] = D_80040E38[0].b[1] = D_80040E38[1].b[1] = 0;
            D_800419B8[0] = D_80031BD4;
            D_800419B8[1] = D_80031BD4;
            D_80040E60[0] = D_80031A94;
            D_80040E60[1] = D_80031A94;
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    D_8004AC60[0][i][j] = D_80031A64[i][j];
                    D_8004AC60[1][i][j] = D_80031A64[i][j];
                }
            }
            for (i = 0; i < 12; i++) {
                D_80040830[0][i] = D_80031A58[i];
                D_80040830[1][i] = D_80031A58[i];
            }
            D_80039D1C[0] = D_80039D1C[1] = 0;
            D_80039D08[0] = D_80039D08[1] = D_80039D08[2] = D_80039D08[3] = 0;
            D_80039D28[0] = D_80039D28[1] = D_80039CA2;
            D_80039D12[0] = D_80039D12[1] = D_80039CA3;
        }
        func_8005D39C(2);
        func_80051450(4, 1);
        func_80051450(4, 4);
        o->update = func_800A8FFC;
    }
}
