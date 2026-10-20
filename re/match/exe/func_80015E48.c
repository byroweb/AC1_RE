/* exe 0x80015E48  size 32  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80015E48(void *arg0) {
    s32 *temp_v0;

    temp_v0 = M2C_FIELD(arg0, s32 **, 8);
    M2C_FIELD(arg0, s8 *, 0) = 0;
    if (temp_v0 != NULL) {
        *temp_v0 = 0;
        M2C_FIELD(arg0, s32 **, 8) = NULL;
    }
}
