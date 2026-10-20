/* exe 0x800162A0  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 GetRCnt(M2C_UNK);                               /* extern */

s32 func_800162A0(s32 arg0) {
    s32 temp_v1;
    s32 var_v0;

    temp_v1 = GetRCnt(0xF2000001) - arg0;
    var_v0 = temp_v1;
    if (temp_v1 < 0) {
        var_v0 = temp_v1 + 0x10000;
    }
    return var_v0;
}
