/* ovl202 0x80076350  size 164  permuter tier (tools/loop/permute.py, decomp-permuter): from ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004ECA8(s32);                         /* extern */

void func_80076350(void *arg0)
{
  int new_var;
  M2C_UNK (*temp_v0_2)(void *, u8);
  u16 temp_v0_3;
  u8 temp_v0;
  func_8004ECA8(arg0 + 0x64);
  temp_v0 = (*((u8 *) (((s8 *) arg0) + 0x66))) - 1;
  *((u8 *) (((s8 *) arg0) + 0x66)) = temp_v0;
  if (!(temp_v0 & 0xFF))
  {
    temp_v0_2 = *((M2C_UNK (**)(void *, u8)) (((s8 *) arg0) + 0x50));
    if (temp_v0_2 != ((void *) 0))
    {
      temp_v0_2(arg0, *((u8 *) (((s8 *) arg0) + 0x41)));
    }
  }
  temp_v0_3 = (*((u16 *) (((s8 *) arg0) + 0xCC))) + (*((u16 *) (((s8 *) arg0) + 0xCE)));
  *((u16 *) (((s8 *) arg0) + 0xCC)) = temp_v0_3;
  if (((s16) temp_v0_3) < (-0x40))
  {
    new_var = -0x40U;
    *((u16 *) (((s8 *) arg0) + 0xCC)) = new_var;
  }
  *((u16 *) (((s8 *) arg0) + 0x12)) = (u16) ((*((u16 *) (((s8 *) arg0) + 0x12))) + (*((u16 *) (((s8 *) arg0) + 0xCC))));
}
