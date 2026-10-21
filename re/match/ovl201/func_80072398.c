#include "ac1/ac1.h"
/* ovl201 0x80072398 - once the func_8005BA2C object is gone, open
 * func_800810F0 on func_80070A44 with mode 80 (button 0x100 with
 * D_80039D20 clear, or the 0x2083 combination) or 65, and park. */

extern u32 D_801A2568;
extern u8 D_80039D20;
extern void func_8005BA2C(void);
extern void func_80070A44(void);
extern void func_800810F0(UiObj *o, void (*fn)(void), s32 mode);

void func_80072398(UiObj *o)
{
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        if (((D_801A2568 & 0x100) && D_80039D20 == 0) || (D_801A2568 & 0x2083) == 0x2083) {
            func_800810F0(o, func_80070A44, 80);
        } else {
            func_800810F0(o, func_80070A44, 65);
        }
        o->update = ui_obj_nop2;
    }
}
