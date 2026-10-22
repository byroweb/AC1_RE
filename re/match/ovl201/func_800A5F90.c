#include "ac1/ac1.h"

extern void func_8005BA2C();
extern void func_8005D71C(void);
extern void func_80072838(void);
extern void func_800A6034(UiObj *);
extern RECT D_8004D33C;

void func_800A5F90(UiObj *o)
{
    RECT r;

    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005D71C();
        r = D_8004D33C;
        ClearImage(&r, 0, 0, 0);
        DrawSync(0);
        func_80072838();
        o->update = func_800A6034;
    }
}
