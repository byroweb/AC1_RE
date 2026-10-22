#include "ac1/ac1.h"

extern void func_8005BA2C(s32, s32, s32, s32, s32, s32, s32);
extern void func_8005C2D0();
extern void func_800AC66C(UiObj *);
extern u8 D_80039CA2;
extern s16 D_801A2318;

void func_800AC58C(UiObj *o)
{
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_8005BA2C(0xB0, 0x18, 0x80, 0x90, 5, 1, 0);
        if (D_80039CA2 == 0) {
            func_8005BA2C(0x20, 0x28, 0x88, 0x90, 3, 0, 0);
        } else {
            func_8005BA2C(0x20, 0x28, 0x88, 0x80, 3, 0, 0);
        }
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        o->update = func_800AC66C;
    }
}
