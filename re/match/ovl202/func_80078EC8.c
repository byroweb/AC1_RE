/* ovl202 0x80078EC8  size 56  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80078D78(AcEntity *, void *);          /* extern */

void func_80078EC8(void *arg0) {
    AcEntity *temp_v0;

    temp_v0 = ac_entity_find_def(arg0);
    if (temp_v0 != NULL) {
        func_80078D78(temp_v0, arg0);
    }
}
