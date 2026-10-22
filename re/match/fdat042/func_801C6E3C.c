/* fdat042 0x801C6E3C  size 92  permuter tier (tools/loop/permute.py): decomp-permuter, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004E98C(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK func_801C6D64;

void func_801C6E3C(void *arg0)
{
  int new_var;
  *((s16 *) (((s8 *) arg0) + 0x36)) = 1;
  *((M2C_UNK **) (((s8 *) arg0) + 0x58)) = &func_801C6D64;
  new_var = 0xC8;
  *((s32 *) (((s8 *) arg0) + 0x50)) = 0;
  *((s32 *) (((s8 *) arg0) + 0x54)) = 0;
  *((s32 *) (((s8 *) arg0) + 0x5C)) = 0;
  *((s8 *) (((s8 *) arg0) + 0x40)) = new_var;
  *((s8 *) (((s8 *) arg0) + 0x42)) = 0;
  *((s16 *) (((s8 *) arg0) + 0x44)) = 0;
  func_8004E98C(arg0 + 0x64, 1, 0, 0);
}
