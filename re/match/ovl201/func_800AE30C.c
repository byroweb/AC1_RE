/* ovl201 0x800AE30C  size 100  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800AE370(UiObj *);                        /* extern */
extern s8 D_801A2308;
extern u8 D_801A230B;

void func_800AE30C(void) {
    UiObj *temp_v0;

    D_801A2308 = 1;
    temp_v0 = ui_obj_alloc((s32) D_801A230B);
    temp_v0->ctor = func_800AE30C;
    temp_v0->update = func_800AE370;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
}
