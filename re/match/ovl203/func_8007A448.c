/* ovl203 0x8007A448  size 176  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004E0A4(void *, void *);              /* extern */
M2C_UNK func_80076338(void *, M2C_UNK, M2C_UNK, M2C_UNK, void *, M2C_UNK *); /* extern */
extern void *D_80039D18;
extern M2C_UNK func_8007A09C;

void func_8007A448(void *arg0, M2C_UNK arg1) {
    if (M2C_FIELD(arg0, u8 *, 0x3D) != 0xB0) {
        func_8004E0A4(arg0 + 0x64, arg0 + 0xCC);
        M2C_FIELD(arg0, s16 *, 0xCE) = 0;
        M2C_FIELD(arg0, u16 *, 0xCC) = (u16) (M2C_FIELD(arg0, u16 *, 0xCC) << 7);
        M2C_FIELD(arg0, u16 *, 0xD0) = (u16) (M2C_FIELD(arg0, u16 *, 0xD0) << 7);
    }
    M2C_FIELD(arg0, s32 *, 0x15C) = (s32) (M2C_FIELD(arg0, s32 *, 0x15C) & M2C_FIELD(D_80039D18, u16 *, 0x2BE));
    func_80076338(arg0, arg1, 1, 0x200, arg0 + 0xCC, &func_8007A09C);
    M2C_FIELD(arg0, s8 *, 0x3C) = 0x41;
    M2C_FIELD(arg0, s8 *, 0x41) = 0x41;
}
