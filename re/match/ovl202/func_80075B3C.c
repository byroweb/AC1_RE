/* ovl202 0x80075B3C  size 132  permuter tier (tools/loop/permute.py): decomp-permuter, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80075B3C(void *arg0)
{
  s16 temp_v0;
  volatile unsigned char pad;
  int new_var2;
  int new_var;
  s16 temp_v0_2;
  temp_v0 = *((s16 *) (((s8 *) arg0) + 0xCC));
  if (temp_v0 > 0)
  {
    if (temp_v0 >= 0x101)
    {
      *((s16 *) (((s8 *) arg0) + ((unsigned short) 0xCC))) = (s16) (temp_v0 - 0x100);
    }
    else
    {
      goto block_5;
    }
  }
  else
    if (temp_v0 < (-0x100))
  {
    *((s16 *) (((s8 *) arg0) + 0xCC)) = (s16) (temp_v0 + 0x100);
  }
  else
  {
    block_5:
    *((s16 *) (((s8 *) arg0) + 0xCC)) = 0;

  }
  temp_v0_2 = *((s16 *) (((s8 *) arg0) + 0xD0));
  new_var = -0x100;
  if (temp_v0_2 > 0)
  {
    if (temp_v0_2 >= 0x101)
    {
      *((s16 *) (((s8 *) arg0) + 0xD0)) = (s16) (temp_v0_2 - 0x100);
      return;
    }
    goto block_11;
  }
  if (temp_v0_2 < new_var)
  {
    *((s16 *) (((s8 *) arg0) + (new_var2 = 0xD0))) = (s16) (temp_v0_2 + 0x100);
    return;
  }
  block_11:
  *((s16 *) (((s8 *) arg0) + 0xD0)) = 0;

}
