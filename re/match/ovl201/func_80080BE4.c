/* ovl201 0x80080BE4  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
s32 func_800517C8();                                /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern M2C_UNK func_80080C58;

void func_80080BE4(void *arg0) {
    if ((func_800517C8() << 0x10) == 0) {
        func_800506EC(0xA);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0x10, 0x78, 0x118, 0x20, 9);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80080C58;
    }
}
