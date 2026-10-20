/* ovl201 0x8008E70C  size 212  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_8005CAE0(M2C_UNK *, M2C_UNK, s32, M2C_UNK); /* extern */
void func_8008E7E0(UiObj *);                        /* extern */
extern M2C_UNK D_800B91AC;
extern M2C_UNK D_800B91B8;
extern s8 D_8019C094;

UiObj *func_8008E70C(void) {
    UiObj *temp_v0;
    void *temp_v0_2;
    void *temp_v0_3;

    temp_v0 = ui_obj_alloc(0xA);
    temp_v0->ctor = func_8008E70C;
    temp_v0->update = func_8008E7E0;
    temp_v0->destroy = ui_obj_nop;
    temp_v0->visible = 0;
    temp_v0->attached = 0;
    temp_v0->parent = NULL;
    temp_v0->has_xform = 0;
    temp_v0_2 = func_8005CAE0(&D_800B91AC, 0x72, 0x1A, 0x12);
    M2C_FIELD(temp_v0, void **, 0x8C) = temp_v0_2;
    M2C_FIELD(temp_v0_2, s16 *, 0x16) = 0xB;
    M2C_FIELD(M2C_FIELD(temp_v0, void **, 0x8C), s8 *, 2) = 0;
    temp_v0_3 = func_8005CAE0(&D_800B91B8, 0x72, (D_8019C094 * 0x18) + 8, 0x12);
    M2C_FIELD(temp_v0, void **, 0x90) = temp_v0_3;
    M2C_FIELD(temp_v0_3, s16 *, 0x16) = 0xB;
    M2C_FIELD(M2C_FIELD(temp_v0, void **, 0x90), s8 *, 2) = 0;
    return temp_v0;
}
