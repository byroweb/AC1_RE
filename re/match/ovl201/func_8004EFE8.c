/* ovl201 0x8004EFE8  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8004DF10(UiObj *);                        /* extern */
extern M2C_UNK func_8004DCC0;

void func_8004EFE8(void) {
    UiObj *temp_v0;

    temp_v0 = obj_find_by_handle(&func_8004DCC0);
    M2C_FIELD(M2C_FIELD(temp_v0, void **, 0x98), s8 *, 1) = 1;
    temp_v0->update = func_8004DF10;
}
