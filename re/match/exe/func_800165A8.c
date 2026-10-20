/* exe 0x800165A8  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80016328(void *, s32, u32);            /* extern */

void func_800165A8(s32 arg0, void *arg1, s32 arg2) {
    s32 var_a2;

    var_a2 = arg2;
    if (var_a2 == 0) {
        var_a2 = M2C_FIELD(arg1, s32 *, 4);
    }
    func_80016328(arg1, arg0, (u32) (var_a2 + 0x7FF) >> 0xB);
}
