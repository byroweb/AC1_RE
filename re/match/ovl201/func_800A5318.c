#include "ac1/ac1.h"
extern s8 D_80039CAC;
extern s8 D_80039CAD;
extern void func_800A5368(void);
extern void func_800A5414(UiObj *o);

void func_800A5318(UiObj *o)
{
    UiObj *p = *(UiObj **)&o->w.raw[4];

    D_80039CAC = 0;
    D_80039CAD = 1;
    p->kill = 1;
    func_800A5368();
    o->update = func_800A5414;
}
