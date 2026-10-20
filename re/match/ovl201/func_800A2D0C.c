/* ovl201 0x800A2D0C  size 176  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051450(M2C_UNK, M2C_UNK);            /* extern */
void func_800A2DBC(UiObj *);                        /* extern */
void func_800A304C(UiObj *);                        /* extern */
extern s8 D_800BDBBC;
extern s8 D_800BDBBD;

UiObj *func_800A2D0C(u8 arg0) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0);
    temp_v0->ctor = func_800A2D0C;
    temp_v0->update = func_800A2DBC;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->w.raw[0x23] = arg0;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->has_xform = 0;
    D_800BDBBC = 0;
    D_800BDBBD = 1;
    if (!(arg0 & 0xFF)) {
        temp_v0->w.raw[0x21] = 0;
        func_80051450(4, 2);
    } else {
        temp_v0->update = func_800A304C;
    }
    return temp_v0;
}
