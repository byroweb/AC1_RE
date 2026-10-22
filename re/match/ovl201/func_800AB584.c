#include "ac1/ac1.h"

extern s16 func_800517C8(void);
extern void func_800A9068(void);
extern void func_800A9AFC(UiObj *);
extern void func_800AB8D8(void);
extern u8 D_80039CAE;
extern s16 D_801A2318;
extern u32 D_801A2550;
extern u32 D_801A2568;

void func_800AB584(UiObj *o)
{
    UiWork *w = &o->w;
    u8 busy = func_800517C8();

    if ((D_801A2568 & 0x2000) && !busy) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (++D_80039CAE == 85) {
            D_80039CAE = 80;
        }
        func_800AB8D8();
    } else if ((D_801A2568 & 0x8000) && !busy) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (--D_80039CAE == 79) {
            D_80039CAE = 84;
        }
        func_800AB8D8();
    } else if ((D_801A2568 & 0x20000000) && !busy) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (++D_80039CAE == 85) {
            D_80039CAE = 80;
        }
        func_800AB8D8();
    } else if ((D_801A2568 & 0x80000000) && !busy) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        if (--D_80039CAE == 79) {
            D_80039CAE = 84;
        }
        func_800AB8D8();
    } else if (D_801A2568 & 0x400040) {
        if (D_801A2568 & 0x40) {
            D_801A2550 |= 0x40;
        }
        if (D_801A2568 & 0x400000) {
            D_801A2550 |= 0x400000;
        }
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        w->ring.item[1]->kill = 1;
        w->ring.item[2]->kill = 1;
        o->update = func_800A9AFC;
        while (func_800517C8() != 0) {
        }
        func_800A9068();
    } else if (D_801A2568 & 0x200020) {
        if (D_801A2568 & 0x20) {
            D_801A2550 |= 0x20;
        }
        if (D_801A2568 & 0x200000) {
            D_801A2550 |= 0x200000;
        }
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        D_80039CAE = o->w.raw[6];
        func_800AB8D8();
        while (func_800517C8() != 0) {
        }
        func_800A9068();
        w->ring.item[1]->kill = 1;
        w->ring.item[2]->kill = 1;
        o->update = func_800A9AFC;
    }
}
