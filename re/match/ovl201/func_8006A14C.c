/* ovl201 0x8006A14C  size 376  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_8005CAE0(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8006A2C4(void *);                      /* extern */
s32 func_8006AE88(s32, s32);                        /* extern */
M2C_UNK func_8006BA08();                            /* extern */
extern M2C_UNK D_800B91AC;
extern M2C_UNK D_800B91B8;
extern M2C_UNK D_800B91C4;
extern M2C_UNK D_800B91D0;
extern M2C_UNK D_800B91DC;
extern M2C_UNK D_800B91E8;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_8006A3EC;

void func_8006A14C(void *arg0) {
    void *temp_v0;
    void *temp_v0_2;
    void *temp_v0_3;
    void *temp_v0_4;
    void *temp_v0_5;
    void *temp_v0_6;

    func_8006BA08();
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        M2C_FIELD(arg0, s8 *, 0xC5) = 0;
        M2C_FIELD(arg0, s8 *, 0xC6) = 0;
        M2C_FIELD(arg0, s8 *, 0xC7) = 0;
        M2C_FIELD(arg0, s16 *, 0xC8) = 0;
        M2C_FIELD(arg0, s32 *, 0xA8) = func_8006AE88(arg0 + 0xC6, arg0 + 0xC7);
        func_8006A2C4(arg0);
        temp_v0 = func_8005CAE0(&D_800B91C4, 0x18, 0xC0, 0x14);
        M2C_FIELD(arg0, void **, 0xB4) = temp_v0;
        M2C_FIELD(temp_v0, s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0xB4), s8 *, 2) = 0;
        temp_v0_2 = func_8005CAE0(&D_800B91E8, 0x28, 0xC4, 0x14);
        M2C_FIELD(arg0, void **, 0xB0) = temp_v0_2;
        M2C_FIELD(temp_v0_2, s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0xB0), s8 *, 2) = 0;
        temp_v0_3 = func_8005CAE0(&D_800B91D0, 0x11C, 0xC0, 0x14);
        M2C_FIELD(arg0, void **, 0xB8) = temp_v0_3;
        M2C_FIELD(temp_v0_3, s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0xB8), s8 *, 2) = 0;
        temp_v0_4 = func_8005CAE0(&D_800B91DC, 0x10C, 0xC4, 0x14);
        M2C_FIELD(arg0, void **, 0xAC) = temp_v0_4;
        M2C_FIELD(temp_v0_4, s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0xAC), s8 *, 2) = 0;
        temp_v0_5 = func_8005CAE0(&D_800B91AC, 0x96, 0x20, 0x14);
        M2C_FIELD(arg0, void **, 0xBC) = temp_v0_5;
        M2C_FIELD(temp_v0_5, s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0xBC), s8 *, 2) = 0;
        temp_v0_6 = func_8005CAE0(&D_800B91B8, 0x96, 0xC0, 0x14);
        M2C_FIELD(arg0, void **, 0xC0) = temp_v0_6;
        M2C_FIELD(temp_v0_6, s16 *, 0x16) = 0xB;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0xC0), s8 *, 2) = 0;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8006A3EC;
    }
}
