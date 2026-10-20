/* ovl201 0x80090B90  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80090BFC(UiObj *);                        /* extern */

void func_80090B90(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(8);
    temp_v0->ctor = func_80090B90;
    temp_v0->update = func_80090BFC;
    temp_v0->destroy = ui_obj_nop;
    M2C_FIELD(temp_v0, s16 *, 0xE) = 0x1D;
    temp_v0->light = 0x12;
    temp_v0->render_type = 0;
    M2C_FIELD(temp_v0, s16 *, 0xC) = 0;
    temp_v0->scaled = 1;
    temp_v0->rot.vx = 0;
    temp_v0->rot.vy = 0;
    temp_v0->rot.vz = 0;
}
