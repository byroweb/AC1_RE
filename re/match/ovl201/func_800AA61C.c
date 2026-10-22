#include "ac1/ac1.h"

typedef struct { u8 b[7]; } Blk7;

extern u8 func_800714FC(void *, void *);
extern void func_8005DED8(UiObj *, void (*)(UiObj *), void *, s32);
extern void func_800AA710(UiObj *);
extern u8 D_80031A58[];
extern u8 D_8004083C[];
extern Blk7 D_80039D20;
extern Blk7 D_80040E3F;
extern u8 D_80040874[];
extern u8 D_800419DC[];
extern u8 D_8004D430[];

void func_800AA61C(UiObj *o)
{
    s16 i;

    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_8004083C[i];
    }
    D_80039D20 = D_80040E3F;
    if (func_800714FC(D_80040874, D_800419DC) == 0) {
        func_8005DED8(o, func_800AA710, D_8004D430, 12);
        o->w.raw[3] = 0;
        o->update = ui_obj_nop2;
    } else {
        o->update = func_800AA710;
    }
}
