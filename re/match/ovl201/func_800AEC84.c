/* ovl201 0x800AEC84  size 600  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
void *func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_8005DD38(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8005DE08(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_800AE99C(void *);                      /* extern */
M2C_UNK func_800AEEDC(void *);                      /* extern */
extern M2C_UNK D_8004D844;
extern M2C_UNK D_8004D854;
extern M2C_UNK D_8004D860;
extern M2C_UNK D_8004D870;
extern M2C_UNK D_8004D87C;
extern M2C_UNK D_8004D88C;
extern M2C_UNK D_800BDDDC;
extern M2C_UNK func_80052134;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800AEF2C;

void func_800AEC84(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005DD38(0x3C0, 0x80, 0xF);
        func_8005DE08(0x3C0, 0x98, 0xF);
        func_8005DE08(0x3C0, 0xB0, 0x11);
        DrawSync(0);
        func_8005D754(3, &D_800BDDDC, 0, 0, 4);
        func_8005D754(3, &D_800BDDDC, 0, 0x18, 4);
        func_8005D754(3, &D_800BDDDC, 0, 0x30, 4);
        func_8005D754(3, &D_800BDDDC, 0, 0x48, 4);
        func_8005D754(3, &D_800BDDDC, 0, 0x60, 4);
        func_8005D754(3, &D_800BDDDC, 0, 0x78, 4);
        func_8005D80C(2, &D_8004D844, 0x10, 8, 4, 4);
        func_8005D80C(2, &D_8004D854, 0x10, 8, 0x1C, 4);
        if (M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s32 *, 0x70) != &func_80052134) {
            M2C_FIELD(func_8005D80C(2, &D_8004D860, 0x10, 8, 0x34, 4), s16 *, 0x16) = 0x10;
        } else {
            func_8005D80C(2, &D_8004D860, 0x10, 8, 0x34, 4);
        }
        func_8005D80C(2, &D_8004D870, 0x10, 8, 0x4C, 4);
        func_8005D80C(2, &D_8004D87C, 0x10, 8, 0x64, 4);
        func_8005D80C(2, &D_8004D88C, 0x10, 8, 0x7C, 4);
        func_800AEEDC(arg0);
        func_800AE99C(arg0 + 0x88);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800AEF2C;
    }
}
