/* ovl201 0x8005D39C  size 148  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800514AC(s32);                         /* extern */
void func_8005D430(UiObj *);                        /* extern */
extern s8 D_801BC471;
extern s8 D_801BC472;
extern s8 D_801BC473;
extern s8 D_801BC474;

UiObj *func_8005D39C(s32 arg0) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0);
    temp_v0->ctor = func_8005D39C;
    temp_v0->update = func_8005D430;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    func_800514AC(arg0 & 0xFFFF);
    D_801BC473 = 1;
    D_801BC472 = 1;
    D_801BC471 = 1;
    D_801BC474 = 1;
    return temp_v0;
}
