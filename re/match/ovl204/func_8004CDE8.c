/* ovl204 0x8004CDE8  size 228  permuter tier (tools/loop/permute.py): decomp-permuter after levers bytes &D_80041B90 + E, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8004CCC0(M2C_UNK, M2C_UNK *, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_8004D278(s32, M2C_UNK, s32, s32, s32); /* extern */
extern M2C_UNK D_80041B90;

s32 func_8004CDE8(s32 arg0, M2C_UNK arg1, M2C_UNK arg2, M2C_UNK arg3, s32 arg4, s32 arg5, s32 arg6)
{
  s32 new_var;
  s32 temp_s0;
  s32 temp_v0;
  int new_var2;
  new_var2 = arg0 & 0xFFF;
  new_var = arg4;
  temp_s0 = func_8004CCC0(arg2, &D_80041B90, arg3, new_var, arg5);
  temp_v0 = func_8004CCC0(arg2, (M2C_UNK *) (((u8 *) (&D_80041B90)) + 0x14), arg3, arg4, arg5);
  if ((temp_s0 == 0) && (temp_v0 == 0))
  {
    return 0;
  }
  func_8004D278(new_var2, arg1, temp_s0, temp_v0, arg6);
  return 1;
}
