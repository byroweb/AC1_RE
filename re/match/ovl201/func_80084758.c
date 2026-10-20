/* ovl201 0x80084758  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
M2C_UNK func_80083FA0(s32);                         /* extern */
s32 func_80084290();                                /* extern */
s32 func_80085258(s32);                             /* extern */
extern M2C_UNK func_80084AE4;

void func_80084758(void *arg0) {
    if ((func_800517C8() << 0x10) == 0) {
        func_80083FA0(arg0 + 0xC4);
        M2C_FIELD(arg0, s32 *, 0xC0) = func_80085258(func_80084290() & 0xFF);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80084AE4;
    }
}
