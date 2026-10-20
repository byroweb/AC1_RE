/* ovl201 0x80068E14  size 424  permuter tier (tools/loop/permute.py): decomp-permuter after levers swap func_8006BA08(); / var_s0 = 0;, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, s16, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_8006BA08();                            /* extern */
M2C_UNK func_800702C8(M2C_UNK, s32, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_8004B728;
extern M2C_UNK D_8004B730;
extern M2C_UNK D_8004B738;
extern M2C_UNK D_8004B740;
extern M2C_UNK D_8004B748;
extern M2C_UNK D_8004B750;
extern M2C_UNK D_800B7FAC;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_80068FBC;

void func_80068E14(void *arg0)
{
  s16 temp_v0;
  s16 var_s0;
  func_8006BA08();
  if (obj_find_by_handle(&func_8005BA2C) == (&g_ui_none))
  {
    var_s0 = 0;
    do
    {
      func_8005D754(3, &D_800B7FAC, 0, (s16) (var_s0 * 0x18), 0xA);
      temp_v0 = var_s0 + 1;
      var_s0 = temp_v0;
    }
    while (temp_v0 < 6);
    func_8005D80C(2, &D_8004B728, 0x10, 8, 4, 0xA);
    func_8005D80C(2, &D_8004B730, 0x10, 8, 0x1C, 0xA);
    func_8005D80C(2, &D_8004B738, 0x10, 8, 0x34, 0xA);
    func_8005D80C(2, &D_8004B740, 0x10, 8, 0x4C, 0xA);
    func_8005D80C(2, &D_8004B748, 0x10, 8, 0x64, 0xA);
    func_8005D80C(2, &D_8004B750, 0x10, 8, 0x7C, 0xA);
    func_800702C8(0xA, arg0 + 0x90, 0, 0);
    *((M2C_UNK **) (((s8 *) arg0) + 0x74)) = &func_80068FBC;
    *((u8 *) (((s8 *) arg0) + 0x8D)) = (u8) (*((u8 *) (((s8 *) arg0) + 0x8C)));
  }
}
