/* ovl201 0x80053C98  size 284  permuter tier (tools/loop/permute.py): decomp-permuter after levers swap var_v1 += 1; / var_v0 = var_v1 & 0xFF;, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32, s32); /* extern */
extern s16 D_801A2318;
extern u8 D_801A257C;
extern M2C_UNK func_80053DB4;
extern M2C_UNK func_8005C2D0;

void func_80053C98(void *arg0)
{
  u8 var_v1;
  if (obj_find_by_handle(&func_8005C2D0) == (&g_ui_none))
  {
    func_8005BA2C(0x10, 0x28, 0x60, 0x18, 3, 1, 0);
    func_8005BA2C(0x10, 0x40, 0x120, 0x18, 5, 1, 0);
    var_v1 = D_801A257C;
    var_v1 = var_v1 & 0xFF;
    if ((*((u8 *) (((s8 *) arg0) + 0xA7))) != 0)
    {
      var_v1 = var_v1 & 0xFF;
      var_v1 += 1;
    }
    if (var_v1 >= 6U)
    {
      var_v1 = 5;
    }
    func_8005BA2C(0x10, 0x58, 0x120, (var_v1 & 0xFF) * 0x18, 7, 0, 0);
    SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
    *((M2C_UNK **) (((s8 *) arg0) + 0x74)) = &func_80053DB4;
  }
}
