/* ovl201 0x80072838  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800728A8(UiObj *);                        /* extern */
void *func_800728D8(UiObj *);                       /* extern */

UiObj *func_80072838(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0x15);
    temp_v0->ctor = func_80072838;
    temp_v0->update = func_800728A8;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    temp_v0->w.list.src = func_800728D8(temp_v0);
    return temp_v0;
}
