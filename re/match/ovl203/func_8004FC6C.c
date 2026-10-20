/* ovl203 0x8004FC6C  size 172  variant sweep (tools/match/variants.py): args lever func_8004FB8C +arg0,arg1, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004FB8C(s32, u16 *);                            /* extern */

void func_8004FC6C(s32 arg0, u16 *arg1, u8 *arg2, u8 *arg3, u8 *arg4, u8 *arg5) {
    func_8004FB8C(arg0, arg1);
    if ((*arg1 & 0x1F) == 0x18) {
        *arg2 += 0x40;
        *arg3 += 0x40;
        *arg4 += 0x40;
        *arg5 += 0x40;
    }
}
