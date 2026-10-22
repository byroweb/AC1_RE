#include "ac1/ac1.h"

extern void func_800A6EA0(UiObj *o);
extern void func_800A57F0(UiObj *o);

void func_800A6E54(UiObj *o)
{
    u8 s;

    s = o->w.raw[0x88 - 0x7C];
    if (s == 0) {
        func_800A6EA0(o);
    } else if (s == 1) {
        o->update = func_800A57F0;
    }
}
