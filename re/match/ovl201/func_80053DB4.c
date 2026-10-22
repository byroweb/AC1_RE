#include "ac1/ac1.h"

extern void func_80051C44(u8 *p, s32 c);
extern void func_80053E98(UiObj *o);
extern void func_800542F0(UiObj *o);
extern void func_8005D80C(s32 a, void *s, s32 b, s32 c, s32 x, s32 y);
extern u8 D_8004AF88[];
extern u8 D_8004AF94[];
extern u8 D_8004B0CC[];
extern void func_8005BA2C(void);

void func_80053DB4(UiObj *o)
{
    u8 *w;
    UiObj *f;

    f = obj_find_by_handle(func_8005BA2C);
    w = o->w.raw;
    if (f == &g_ui_none) {
        if (o->w.raw[0x29] == 'S') {
            func_8005D80C(2, D_8004AF88, 0x10, 8, 4, 4);
        } else {
            func_8005D80C(2, D_8004AF94, 0x10, 8, 4, 4);
        }
        func_8005D80C(2, D_8004B0CC, 0x10, 8, 4, 6);
        func_80051C44(w + 0x26, 'L');
        func_80051C44(w + 0x26, 'R');
        func_80053E98(o);
        o->update = func_800542F0;
    }
}
