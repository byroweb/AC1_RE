#include "ac1/ac1.h"

extern s16 func_800517C8(void);
extern void func_8005CAE0(void *, s32, s32, s32);
extern void func_800A9614(void);
extern void func_800A96E8(UiObj *);
extern u8 D_800BDD7C[];
extern void *D_801F36D0[];
extern s32 D_801BC478[];
extern void *D_80041A88[];
extern s16 D_80041894[];
extern u8 D_80039CAB;

void func_800A953C(UiObj *o)
{
    s16 i;
    UiObj *p;

    if (func_800517C8() == 0) {
        func_8005CAE0(D_800BDD7C, 16, 16, 2);
        for (i = 3; i < 15; i++) {
            D_80041A88[i] = D_801F36D0[i];
            D_80041894[i] = D_801BC478[i];
        }
        p = *(UiObj **)((u8 *)o + 0x8C);
        D_80039CAB = 1;
        p->kill = 1;
        func_800A9614();
        o->update = func_800A96E8;
    }
}
