/* ovl201 0x800B04BC  size 220  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
void func_800B0598(UiObj *);                        /* extern */
extern s16 D_801A2318;

UiObj *func_800B04BC(s32 arg0, s32 arg1) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0);
    temp_v0->ctor = func_800B04BC;
    temp_v0->update = func_800B0598;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    temp_v0->w.raw[1] = 0;
    temp_v0->w.raw[0] = 0;
    M2C_FIELD(temp_v0, s32 *, 0x80) = arg0;
    M2C_FIELD(temp_v0, s32 *, 0x84) = arg1;
    func_8005BA2C(0xC, 0x30, 0x128, 0xA0, 7, 1, 0);
    SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
    return temp_v0;
}
