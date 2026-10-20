/* ovl201 0x800A38CC  size 268  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
void *func_8005CB94(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005DD38(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8005DE08(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_800A3814(s32);                         /* extern */
extern M2C_UNK D_8004D164;
extern M2C_UNK D_8004D170;
extern M2C_UNK D_800BDC44;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800A39D8;

void func_800A38CC(void *arg0) {
    if (((func_800517C8() << 0x10) == 0) && (obj_find_by_handle(&func_8005BA2C) == &g_ui_none)) {
        func_8005DD38(0x3C0, 0x10, 0xF);
        func_8005DE08(0x3C0, 0x28, 0xF);
        func_8005D754(3, &D_800BDC44, 0, 0, 4);
        func_8005D754(3, &D_800BDC44, 0, 0x18, 4);
        func_800A3814(arg0 + 0x9E);
        M2C_FIELD(func_8005CB94(&D_8004D164, 0x10, 4, 4), s16 *, 0x14) = 2;
        M2C_FIELD(func_8005CB94(&D_8004D170, 0x10, 0x1C, 4), s16 *, 0x14) = 2;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A39D8;
    }
}
