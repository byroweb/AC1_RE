#include "ac1/ac1.h"
extern u32 D_801A2568;
extern u32 D_801A256C;
extern void func_80093C3C(UiObj *o);

void func_80093E3C(UiObj *o)
{
    if (((D_801A2568 & 0x1000) && !(D_801A256C & 0x1000)) ||
        ((D_801A2568 & 0x4000) && !(D_801A256C & 0x4000))) {
        o->update = func_80093C3C;
    }
}
