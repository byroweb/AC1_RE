/* ovl201 0x800685A8  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx-regvars draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK _card_info(s32);                            /* extern */
M2C_UNK func_80068828();                            /* extern */
M2C_UNK func_80068930();                            /* extern */

void func_800685A8(s32 arg0) {
    s32 var_s0;

    var_s0 = arg0 & 0xFF;
    var_s0 = var_s0 != 0;
    var_s0 *= 0x10;
    func_80068930();
    _card_info(var_s0);
    func_80068828();
}
