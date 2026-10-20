/* ovl201 0x80086E04  size 48  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80089250(UiObj *);                        /* extern */
extern M2C_UNK func_80088DE4;

void func_80086E04(void) {
    obj_find_by_handle(&func_80088DE4)->update = func_80089250;
}
