#include "ac1/ac1.h"

typedef struct { u8 b[7]; } Blk7;

extern u8 func_800714FC(void *, void *);
extern void func_8005DED8(UiObj *, void (*)(UiObj *), void *, s32);
extern void func_800AA61C(UiObj *);
extern u8 D_80031A58[];
extern u8 D_80040830[];
extern Blk7 D_80039D20;
extern Blk7 D_80040E38;
extern u8 D_80040850[];
extern u8 D_800419B8[];
extern u8 D_8004D40C[];

void func_800AA520(UiObj *o)
{
    s16 i;

    o->w.raw[3] = 1;
    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_80040830[i];
    }
    D_80039D20 = D_80040E38;
    if (func_800714FC(D_80040850, D_800419B8) == 0) {
        func_8005DED8(o, func_800AA61C, D_8004D40C, 12);
        o->w.raw[3] = 0;
        o->update = ui_obj_nop2;
    } else {
        o->update = func_800AA61C;
    }
}
