/* ovl201 0x800AEB30  size 196  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005CC48();                            /* extern */
M2C_UNK func_800AEBF4(UiObj *);                     /* extern */
void func_800AEC84(UiObj *);                        /* extern */
extern s32 D_801A2574;

UiObj *func_800AEB30(void *arg0, s32 arg1) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0);
    temp_v0->ctor = func_800AEB30;
    temp_v0->update = func_800AEC84;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    temp_v0->w.list.src = arg0;
    M2C_FIELD(temp_v0, s32 *, 0x80) = arg1;
    if (obj_find_by_handle(func_8005CC48) == &g_ui_none) {
        temp_v0->w.raw[0xE] = 1;
        func_8005CC48();
    } else {
        temp_v0->w.raw[0xE] = 0;
    }
    func_800AEBF4(temp_v0);
    D_801A2574 = 0;
    return temp_v0;
}
