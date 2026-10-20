/* exe 0x800158C8  size 132  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8001586C(u8);                              /* extern */

s32 func_800158C8(void *arg0) {
    s32 temp_s0;
    s32 temp_s1;

    temp_s1 = func_8001586C(M2C_FIELD(arg0, u8 *, 0));
    temp_s0 = func_8001586C(M2C_FIELD(arg0, u8 *, 1));
    return (temp_s1 * 0x1194) + (temp_s0 * 0x4B) + func_8001586C(M2C_FIELD(arg0, u8 *, 2));
}
