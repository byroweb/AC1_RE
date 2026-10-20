/* ovl202 0x80078CFC  size 124  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80078B14(AcEntity *, u32);             /* extern */
extern M2C_UNK D_8019FB68;

void func_80078CFC(u32 arg0) {
    AcEntity *temp_v0;

    if (arg0 >= (u32) &D_8019FB68) {
        temp_v0 = ac_entity_alloc((void *) arg0);
        if (temp_v0 != NULL) {
            func_80078B14(temp_v0, arg0);
            M2C_FIELD(((temp_v0->pos.w * 4) + g_objective_obj), M2C_UNK (**)(AcEntity *), 0x20)(temp_v0);
        }
    }
}
