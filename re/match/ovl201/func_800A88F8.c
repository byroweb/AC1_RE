/* ovl201 0x800A88F8  size 124  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051450(M2C_UNK, M2C_UNK);            /* extern */
M2C_UNK func_8005CC48();                            /* extern */
M2C_UNK func_800A8974(UiObj *);                     /* extern */
void func_800A89C0(UiObj *);                        /* extern */
extern s32 D_801A2574;

void func_800A88F8(void) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0);
    temp_v0->ctor = func_800A88F8;
    temp_v0->update = func_800A89C0;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    func_800A8974(temp_v0);
    func_80051450(4, 6);
    func_8005CC48();
    D_801A2574 = 2;
}
