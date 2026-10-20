/* ovl201 0x8005489C  size 308  permuter tier (tools/loop/permute.py): decomp-permuter after levers swap var_v1 += 1; / var_v0 = var_v1 << 0x10;, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, s16, s32); /* extern */
extern s16 D_801A2318;
extern u16 D_801A257C;
extern M2C_UNK func_800549D0;

void func_8005489C(void *arg0)
{
  s32 var_v0;
  u16 var_v1;
  SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
  func_800506EC(8);
  func_800506EC(4);
  func_800506EC(6);
  obj_find_by_id(7)->kill = 1;
  obj_find_by_id(3)->kill = 1;
  obj_find_by_id(5)->kill = 1;
  func_8005C2D0(0x10, 0x28, 0x60, 0x18, 3);
  func_8005C2D0(0x10, 0x40, 0x120, 0x18, 5);
  var_v1 = D_801A257C;
  var_v0 = var_v1 << 0x10;
  if ((*((u8 *) (((s8 *) arg0) + 0xA7))) != 0)
  {
    ;
    var_v1 += 1;
  }
  if (((var_v1 << 0x10) >> 0x10) >= 6)
  {
    var_v1 = 5;
  }
  func_8005C2D0(0x10, 0x58, 0x120, (s16) (((s16) var_v1) * 0x18), 7);
  *((M2C_UNK **) (((s8 *) arg0) + 0x74)) = &func_800549D0;
}
