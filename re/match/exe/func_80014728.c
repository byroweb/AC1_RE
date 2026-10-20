/* exe 0x80014728  size 76  permuter tier (tools/loop/permute.py): decomp-permuter, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 rcos(s32);                                      /* extern */
s16 rsin();                                         /* extern */

void func_80014728(s32 arg0, void *arg1)
{
  int new_var;
  if (new_var)
  {
    *((s16 *) (((s8 *) arg1) + 0)) = (s16) (-rsin());
    *((s16 *) (((s8 *) arg1) + 4)) = rcos(arg0);
    new_var = 2;
    *((s16 *) (((s8 *) arg1) + new_var)) = 0;
  }
  else
  {
    *((s16 *) (((s8 *) arg1) + 0)) = (s16) (-rsin());
    *((s16 *) (((s8 *) arg1) + 4)) = rcos(arg0);
    new_var = 2;
    *((s16 *) (((s8 *) arg1) + new_var)) = 0;
  }
}
