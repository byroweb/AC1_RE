/* fdat022 0x801C73A4  size 72  permuter tier (tools/loop/permute.py): decomp-permuter, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_801C705C();                            /* extern */
extern u16 D_8019F528;
extern s32 D_801C761C;

unsigned int func_801C73A4(s32 arg0)
{
  if (((D_8019F528 == 0) && (arg0 == 0)) && (D_801C761C != 3))
  {
    func_801C705C();
  }
}
