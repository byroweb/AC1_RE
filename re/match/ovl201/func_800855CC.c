/* ovl201 0x800855CC  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80085640(UiObj *);                        /* extern */

void func_800855CC(void *arg0, s32 arg1) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0);
    temp_v0->ctor = func_800855CC;
    temp_v0->update = func_80085640;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    temp_v0->w.list.src = arg0;
    M2C_FIELD(temp_v0, s32 *, 0x80) = arg1;
}
