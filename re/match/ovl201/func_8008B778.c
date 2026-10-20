/* ovl201 0x8008B778  size 80  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8008B7C8(UiObj *);                        /* extern */

void func_8008B778(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(8);
    temp_v0->ctor = func_8008B778;
    temp_v0->update = func_8008B7C8;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
}
