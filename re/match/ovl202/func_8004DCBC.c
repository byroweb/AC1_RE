/* ovl202 0x8004DCBC  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004D948(M2C_UNK);                     /* extern */
M2C_UNK func_8004D974();                            /* extern */
extern u8 D_80041BC0;

void func_8004DCBC(void) {
    if (D_80041BC0 == 0) {
        func_8004D948(0);
        return;
    }
    func_8004D974();
}
