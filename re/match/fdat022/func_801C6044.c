/* fdat022 0x801C6044  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_80078A2C();                              /* extern */
M2C_UNK func_80078B14(void *, u32);                 /* extern */
extern M2C_UNK D_8019FB68;

void *func_801C6044(u32 arg0) {
    void *temp_v0;

    if (arg0 >= (u32) &D_8019FB68) {
        temp_v0 = func_80078A2C();
        if (temp_v0 != NULL) {
            func_80078B14(temp_v0, arg0);
            M2C_FIELD(temp_v0, s16 *, 0x36) = 2;
            return temp_v0;
        }
    }
    return NULL;
}
