/* ovl202 0x800745D0  size 80  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80055A24(s32, s32, void *, void *, s32, s32, s32, s32, M2C_UNK *); /* extern */

void func_800745D0(void *arg0, s32 arg1, s32 arg2, s32 arg3, volatile int arg4, M2C_UNK arg5)
{
  func_80055A24(*((s32 *) (((s8 *) arg0) + 0x60)), (*((s16 *) (((s8 *) arg0) + 0xE))) + 0x80, arg0 + 8, arg0 + 0x10, arg1, arg2, arg3, arg4, &arg5);
}
