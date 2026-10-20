/* ovl201 0x8005D8D4  size 276  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
void func_8005D9E8(UiObj *);                        /* extern */
extern s16 D_801A2318;

UiObj *func_8005D8D4(s32 arg0, s8 *arg1, s16 arg2, unsigned char arg3)
{
  UiObj *temp_v0;
  temp_v0 = ui_obj_alloc(0);
  temp_v0->ctor = func_8005D8D4;
  temp_v0->update = func_8005D9E8;
  temp_v0->destroy = ui_obj_nop;
  temp_v0->visible = 0;
  temp_v0->attached = 0;
  temp_v0->parent = (void *) 0;
  temp_v0->has_xform = 0;
  if (!(arg3 & 0xFF))
  {
    *((s16 *) (((s8 *) temp_v0) + 0x88)) = 0;
  }
  else
  {
    *((s16 *) (((s8 *) temp_v0) + 0x88)) = 1;
  }
  *((s32 *) (((s8 *) temp_v0) + 0x8C)) = arg0;
  *((s16 *) (((s8 *) temp_v0) + 0x94)) = arg2;
  *((s8 **) (((s8 *) temp_v0) + 0x90)) = arg1;
  *arg1 = 2;
  func_8005BA2C(0x58, 0x60, 0x90, 0x30, (s32) ((s16) (arg2 - 1)), 0, 0);
  SsUtKeyOn(D_801A2318, 0, 7, 0x3C, 0, 0x7F, 0x7F);
  return temp_v0;
}
