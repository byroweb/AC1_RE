/* ovl201 0x8006F708  size 148  permuter tier (tools/loop/permute.py): decomp-permuter after levers index D_8017E5D8 u8[] + index D_8017E5D9 u8[] + index D_8017E5DA u8[], from the ctx-spframe draft, 2026-10-21 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

struct _m2c_stack_func_8006F708 {};                 /* size 0x0 */

extern u8 D_8017E5D8[];
extern u8 D_8017E5D9[];
extern u8 D_8017E5DA[];
void func_8006F708(void *arg0)
{
  s32 var_v1;
  char temp_a1;
  u8 var_v0;
  void *temp_v1;
  temp_v1 = *((void **) (((s8 *) arg0) + 0x7C));
  temp_a1 = *((u8 *) (((s8 *) temp_v1) + 0x96));
  if (temp_a1 == 0)
  {
    var_v0 = *((u8 *) (D_8017E5D8 + ((*((u8 *) (((s8 *) temp_v1) + 0x86))) * 4)));
    var_v1 = 0xBB;
  }
  else
    if (temp_a1 == 1)
  {
    var_v0 = *((u8 *) (D_8017E5D9 + (2 * (2 * (*((u8 *) (((s8 *) temp_v1) + 0x86)))))));
    var_v1 = 0xC3;
  }
  else
  {
 do { var_v0 = *((u8 *) (D_8017E5DA + ((*((u8 *) (((s8 *) temp_v1) - -0x86))) * 4))); } while (0);
    var_v1 = 0xCB;
  }
  *((s32 *) (((s8 *) arg0) + 0x48)) = (s32) (((var_v0 >> 2) >> 1) + 0x62);
  *((s32 *) (((s8 *) arg0) + 0x4C)) = var_v1;
  *((s32 *) (((s8 *) arg0) + 0x50)) = 0;
}
