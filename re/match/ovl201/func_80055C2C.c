/* ovl201 0x80055C2C  size 180  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004EFE8();                            /* extern */
extern s32 D_801A2574;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_8005CC48;

void func_80055C2C(void *arg0) {
    s32 temp_v1;

    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        if (M2C_FIELD(arg0, u8 *, 0xAB) != 0) {
            obj_find_by_handle(&func_8005CC48)->kill = 1;
        }
        temp_v1 = M2C_FIELD(arg0, s32 *, 0x80);
        if (temp_v1 == 0) {
            D_801A2574 = 0;
            func_8004EFE8();
        } else {
            M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s32 *, 0x74) = temp_v1;
            D_801A2574 = 2;
        }
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
