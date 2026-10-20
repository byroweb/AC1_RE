/* ovl202 0x8007C790  size 56  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004ED5C(void *);                      /* extern */
extern u16 D_8004125E;

void func_8007C790(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x15C) = (s32) (M2C_FIELD(arg0, s32 *, 0x15C) & D_8004125E);
    func_8004ED5C(arg0 + 0x64);
}
