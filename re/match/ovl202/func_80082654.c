/* ovl202 0x80082654  size 168  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800138F0(M2C_UNK, u16 *, s16);         /* extern */
void *func_80071DC4(s16 *, u8);                     /* extern */

s16 func_80082654(void *arg0, M2C_UNK arg1)
{
  union 
  {
    s8 b[0x18];
    s32 w;
  } sp_frame;
  s16 temp_v0_2;
  s16 temp_v0_3;
  void *temp_v0;
  func_800138F0(arg1, &(*((u16 *) (sp_frame.b + 0x0))), *((s16 *) (((s8 *) arg0) + 0x12)));
  temp_v0_2 = (*((u16 *) (((s8 *) arg0) + 8))) + (*((u16 *) (sp_frame.b + 0x0)));
  *((s16 *) (sp_frame.b + 0x10)) = temp_v0_2;
  *((s16 *) (sp_frame.b + 0x8)) = temp_v0_2;
  temp_v0_3 = (*((u16 *) (((s8 *) arg0) + 0xC))) + (*((u16 *) (sp_frame.b + 0x4)));
  *((s16 *) (sp_frame.b + 0x14)) = temp_v0_3;
  *((s16 *) (sp_frame.b + 0xC)) = temp_v0_3;
  *((s16 *) (sp_frame.b + 0xA)) = (*((u16 *) (((s8 *) arg0) + 0xA))) - (((u16) (*((u16 *) (((s8 *) arg0) + 0x2A)))) >> 1);
  *((s16 *) (sp_frame.b + 0x12)) = (*((u16 *) (((s8 *) arg0) + 0xA))) + 0xC8;
  temp_v0 = func_80071DC4(&(*((s16 *) (sp_frame.b + 0x8))), *((u8 *) (((s8 *) arg0) + 0x3A)));
  if (temp_v0 == 0)
  {
    if (sp_frame.b)
    {
      return 0x7FFF;
    }
    else
    {
      return 0x7FFF;
    }
  }
  return *((s16 *) (((s8 *) temp_v0) + 0xE));
}
