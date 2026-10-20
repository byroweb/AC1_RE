/* ovl201 0x80078A38  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
M2C_UNK func_80075898();                            /* extern */
extern M2C_UNK func_80078AAC;

void func_80078A38(void *arg0) {
    u8 *temp_v1;

    func_80075898();
    if (((func_800517C8() << 0x10) == 0) && ((temp_v1 = M2C_FIELD(arg0, u8 **, 0x9C), (temp_v1 == &g_ui_none)) || (*temp_v1 != 1))) {
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80078AAC;
        M2C_FIELD(arg0, u16 *, 0xA6) = (u16) M2C_FIELD(arg0, u16 *, 0xA8);
    }
}
