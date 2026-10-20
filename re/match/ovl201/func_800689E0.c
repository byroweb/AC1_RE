/* ovl201 0x800689E0  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK ClearImage(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8006C1B4(UiObj *);                     /* extern */
extern M2C_UNK D_800B7FC4;

void func_800689E0(void) {
    ClearImage(&D_800B7FC4, 0, 0, 0);
    func_8006C1B4(&g_ui_none);
}
