/* ovl201 0x800828BC  size 312  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_8005CAE0(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_800829F4(void *);                      /* extern */
M2C_UNK func_80083894(s32, s32, M2C_UNK);           /* extern */
extern M2C_UNK D_800B91AC;
extern M2C_UNK D_800B91B8;
extern M2C_UNK D_800B91DC;
extern M2C_UNK D_800B91E8;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_80082E90;

void func_800828BC(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        M2C_FIELD(arg0, s8 *, 0x9C) = 0;
        M2C_FIELD(arg0, s8 *, 0x9D) = 0;
        func_80083894(arg0 + 0x9C, arg0 + 0x9D, 8);
        M2C_FIELD(arg0, void **, 0x84) = func_8005CAE0(&D_800B91AC, 0xB8, 8, 2);
        M2C_FIELD(arg0, void **, 0x88) = func_8005CAE0(&D_800B91B8, 0xB8, 0xC6, 2);
        M2C_FIELD(arg0, void **, 0x8C) = func_8005CAE0(&D_800B91E8, 0xC0, 0x18, 2);
        M2C_FIELD(arg0, void **, 0x90) = func_8005CAE0(&D_800B91DC, 0xC0, 0xB6, 2);
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x84), s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x88), s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x8C), s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x90), s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x84), s8 *, 2) = 0;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x88), s8 *, 2) = 0;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x8C), s8 *, 2) = 0;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x90), s8 *, 2) = 0;
        func_800829F4(arg0);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80082E90;
    }
}
