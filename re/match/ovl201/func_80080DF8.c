#include "ac1/ac1.h"
/* ovl201 0x80080DF8 - once the func_8005BA2C object is gone, blank the eight
 * text lines, put the func_8007C68C object back on func_8007C7D8 (after
 * func_8007C6FC) and retire. */

extern char D_8004C6DC[];
extern void func_8005BA2C(void);
extern void func_8007C68C(void);
extern void func_8007C6FC(UiObj *o);
extern void func_8007C7D8(UiObj *o);
extern void func_8005D15C(char *s, s32 x, s16 y);

void func_80080DF8(UiObj *o)
{
    s16 i;
    UiObj *p;

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        for (i = 0; i < 8; i++) {
            func_8005D15C(D_8004C6DC, 960, i * 16 + 64);
        }
        p = obj_find_by_handle(func_8007C68C);
        func_8007C6FC(p);
        p->update = func_8007C7D8;
        o->kill = 1;
    }
}
