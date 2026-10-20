/* ovl201 0x80088664  size 204  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051628();                            /* extern */
s32 func_800517C8();                                /* extern */
void func_8008AF9C(UiObj *);                        /* extern */
extern s32 D_801A2574;
extern M2C_UNK func_8004DCC0;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_8005CC48;
extern M2C_UNK func_80088730;
extern M2C_UNK func_8008AF0C;

void func_80088664(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        obj_find_by_handle(&func_8008AF0C)->update = func_8008AF9C;
        obj_find_by_handle(&func_8005CC48)->kill = 1;
        D_801A2574 = 0;
        M2C_FIELD(M2C_FIELD(obj_find_by_handle(&func_8004DCC0), void **, 0x98), s8 *, 1) = 1;
        if ((func_800517C8() << 0x10) != 0) {
            func_80051628();
        }
        do {

        } while ((func_800517C8() << 0x10) != 0);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80088730;
    }
}
