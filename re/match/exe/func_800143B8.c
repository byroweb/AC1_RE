/* exe 0x800143B8  size 324  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 SquareRoot0(s32);                               /* extern */

s32 func_800143B8(void *arg0, s32 arg1, s32 arg2, void *arg3)
{
  s16 temp_v0;
  s16 temp_v0_2;
  s32 temp_v0_3;
  s32 var_a0_2;
  s32 var_a1;
  temp_v0 = *((s16 *) (((s8 *) arg0) + 0));
  temp_v0_2 = *((s16 *) (((s8 *) arg0) + 4));
  var_a0_2 = SquareRoot0((temp_v0 * temp_v0) + (temp_v0_2 * temp_v0_2));
  var_a1 = var_a0_2;
  if (arg2 < var_a0_2)
  {
    *((s16 *) (((s8 *) arg3) + 0)) = (s16) (((s32) ((*((s16 *) (((s8 *) arg0) + 0))) * arg2)) / var_a0_2);
    temp_v0_3 = ((s32) ((*((s16 *) (((s8 *) arg0) + 4))) * arg2)) / var_a0_2;
    var_a0_2 = arg2;
    var_a1 = var_a0_2;
    *((s16 *) (((s8 *) arg3) + 4)) = (s16) temp_v0_3;
  }
  var_a0_2 = var_a0_2 - arg1;
  if (var_a0_2 <= 0)
  {
    var_a0_2 = 0;
    *((s16 *) (((s8 *) arg3) + 4)) = 0;
    *((s16 *) (((s8 *) arg3) + 0)) = 0;
  }
  else
  {
    *((s16 *) (((s8 *) arg3) + 0)) = (s16) (((s32) ((*((s16 *) (((s8 *) arg0) + 0))) * var_a0_2)) / var_a1);
    *((s16 *) (((s8 *) arg3) + 4)) = (s16) (((s32) ((*((s16 *) (((s8 *) arg0) + 4))) * var_a0_2)) / var_a1);
  }
  return var_a0_2;
}
