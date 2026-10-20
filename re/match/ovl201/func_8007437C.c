/* ovl201 0x8007437C  size 144  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8007440C(UiObj *);                        /* extern */
extern s16 D_800B9088;
extern s16 D_800B908E;
extern s16 D_800B9094;

void func_8007437C(void *arg0, s32 arg1) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0);
    temp_v0->ctor = func_8007437C;
    temp_v0->update = func_8007440C;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    D_800B9088 = 0x80;
    D_800B908E = 0x80;
    D_800B9094 = 0x80;
    temp_v0->w.list.src = arg0;
    M2C_FIELD(temp_v0, s32 *, 0x80) = arg1;
}
