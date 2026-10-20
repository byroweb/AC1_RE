/* ovl201 0x8005DED8  size 220  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
void func_8005DFB4(UiObj *);                        /* extern */

UiObj *func_8005DED8(void *arg0, s32 arg1, s32 arg2, s16 arg3) {
    UiObj *temp_v0;

    temp_v0 = ui_obj_alloc(0);
    temp_v0->ctor = func_8005DED8;
    temp_v0->update = func_8005DFB4;
    temp_v0->destroy = ui_obj_nop;
    M2C_FIELD(temp_v0, s16 *, 0x8C) = arg3;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    temp_v0->w.list.src = arg0;
    M2C_FIELD(temp_v0, s32 *, 0x80) = arg1;
    M2C_FIELD(temp_v0, s32 *, 0x88) = arg2;
    M2C_FIELD(temp_v0, s16 *, 0x84) = 0;
    func_8005BA2C(0x14, 0x68, 0x118, 0x20, (s32) (s16) (arg3 - 1), 1, 0);
    return temp_v0;
}
