/* ovl202 0x8004FEAC  size 156  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u16 D_80041210;

void func_8004FEAC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, void *arg4)
{
  u16 *new_var;
  new_var = &D_80041210;
  *((s16 *) (((s8 *) arg4) + 0)) = (s16) (((s32) ((((s32) (((-arg1) * arg2) - (arg0 * arg3))) >> 0xC) * 0x17)) / ((s32) (*new_var)));
  *((s16 *) (((s8 *) arg4) + 2)) = (s16) (((s32) ((((s32) ((arg1 * arg3) - (arg0 * arg2))) >> 0xC) * 0x17)) / ((s32) (*new_var)));
}
