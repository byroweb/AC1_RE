/* ovl201 0x80080F40  size 208  permuter tier (tools/loop/permute.py): decomp-permuter, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051450(M2C_UNK, s32);                /* extern */
extern u8 D_80031A58;
extern u8 D_80031A5A;
extern u8 D_80031A5B;
extern u8 D_80031A63;
extern u8 D_80039CA2;
extern u8 D_80039CA3;

void func_80080F40(void)
{
  int new_var;
  func_80051450(0xA, D_80031A5A + (D_80039CA2 * 0xA));
  func_80051450(7, D_80031A58 + (D_80039CA2 * 3));
  func_80051450(9, D_80031A5B + (D_80039CA2 * 0x10));
  func_80051450(8, D_80031A63 + (D_80039CA2 << 5));
  if (D_80039CA2 != 0)
  {
    new_var = (D_80039CA2 * 4) + 0x3A;
    if (D_80039CA2)
    {
      func_80051450(4, D_80039CA3 + new_var);
    }
    else
    {
      func_80051450(4, D_80039CA3 + ((D_80039CA2 * 4) + 0x3A));
    }
  }
}
