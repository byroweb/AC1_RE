/* ovl201 0x8004DEB0  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_8004DF10;
extern M2C_UNK func_8004F294;
extern M2C_UNK func_8004F924;

void func_8004DEB0(void *arg0) {
    if (obj_find_by_handle(&func_8004F924) == &g_ui_none) {
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), M2C_UNK **, 0x74) = &func_8004F294;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8004DF10;
    }
}
