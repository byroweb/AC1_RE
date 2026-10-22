#include "ac1/ac1.h"
extern s16 D_801A2318;
extern UiObj *func_8004DCC0(void);
extern void func_8005BA2C(s32 x, s32 y, s32 w, s32 h, s32 a, s32 b, s32 c);
extern void func_800689E0(void);
extern void func_800A3F90(UiObj *o);

void func_800A3EBC(UiObj *o)
{
    if (o->w.raw[0x20] == 'N') {
        func_8005BA2C(0x18, 0x28, 0x120, 0x88, 3, 1, 0);
        func_8005BA2C(0x18, 0xB2, 0x90, 0x30, 5, 1, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        func_800689E0();
        o->update = func_800A3F90;
        return;
    }
    func_8004DCC0();
    o->kill = 1;
}
