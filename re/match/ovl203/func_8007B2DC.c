/* ovl203 0x8007B2DC  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004EF78(void *);                      /* extern */
extern void *D_80039D18;

void func_8007B2DC(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x15C) = (s32) (M2C_FIELD(arg0, s32 *, 0x15C) & M2C_FIELD(D_80039D18, u16 *, 0x2BE));
    func_8004EF78(arg0 + 0x64);
}
