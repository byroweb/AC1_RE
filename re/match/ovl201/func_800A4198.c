/* ovl201 0x800A4198  size 220  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
M2C_UNK func_8005E92C();                            /* extern */
M2C_UNK func_80073740();                            /* extern */
M2C_UNK func_800866FC();                            /* extern */
M2C_UNK func_80086EC8();                            /* extern */
M2C_UNK func_8008C3DC();                            /* extern */
M2C_UNK func_80096230();                            /* extern */
M2C_UNK func_8009646C();                            /* extern */
M2C_UNK func_8009C964(M2C_UNK);                     /* extern */
extern s8 D_80039C5D;
extern s8 D_80039CAF;
extern s8 D_800411F8;
extern s32 D_80048608;
extern s32 D_8004860C;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_8005CC48;

void func_800A4198(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        obj_find_by_handle(&func_8005CC48)->kill = 1;
        func_80073740();
        do {

        } while ((func_800517C8() << 0x10) != 0);
        func_80086EC8();
        func_8009646C();
        func_8008C3DC();
        func_80096230();
        func_800866FC();
        D_800411F8 = 0;
        D_80039C5D = 0;
        D_8004860C = 0;
        D_80048608 = 0;
        D_80039CAF = 1;
        func_8009C964(0);
        func_8005E92C();
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
