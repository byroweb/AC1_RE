/* ovl202 0x80064C3C  size 124  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80064DC8(s32, M2C_UNK, s32);           /* extern */
M2C_UNK func_800658B8(M2C_UNK);                     /* extern */
M2C_UNK func_8006611C(s32, M2C_UNK, s32, s32);      /* extern */

void func_80064C3C(unsigned short arg0, M2C_UNK arg1, M2C_UNK arg2, s32 arg3, s32 arg4)
{
  s32 temp_a2;
  s32 temp_a0;
  func_800658B8(arg2);
  temp_a2 = arg3 & 0xFFFF;
  temp_a0 = arg0 & 0xFFFF;
  if (temp_a2 == 0)
  {
    func_80064DC8(temp_a0, arg1, arg4);
    return;
  }
  func_8006611C(temp_a0, arg1, temp_a2, arg4);
}
