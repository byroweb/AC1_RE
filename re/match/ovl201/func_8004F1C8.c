/* ovl201 0x8004F1C8  size 204  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK D_800B8E64;
extern u16 D_800B8E6A;
extern u16 D_800B8E6C;
extern u16 D_800B8E70;
extern u16 D_800B8E72;

void func_8004F1C8(void *arg0) {
    if (M2C_FIELD(&D_800B8E64, s16 *, 0) != M2C_FIELD(&D_800B8E64, s16 *, -0x20)) {
        M2C_FIELD(&D_800B8E64, s16 *, 0) = (s16) (M2C_FIELD(&D_800B8E64, s16 *, 0) + 8);
        D_800B8E6A += 8;
        D_800B8E70 += 8;
    }
    if (M2C_FIELD(&D_800B8E64, s16 *, 2) != M2C_FIELD(&D_800B8E64, s16 *, -0x1E)) {
        M2C_FIELD(&D_800B8E64, s16 *, 2) = (s16) (M2C_FIELD(&D_800B8E64, s16 *, 2) + 8);
        D_800B8E6C += 8;
        D_800B8E72 += 8;
    }
    if ((M2C_FIELD(&D_800B8E64, s16 *, 0) == M2C_FIELD(&D_800B8E64, s16 *, -0x20)) && (M2C_FIELD(&D_800B8E64, s16 *, 2) != M2C_FIELD(&D_800B8E64, s16 *, -0x1E))) {
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
    }
}
