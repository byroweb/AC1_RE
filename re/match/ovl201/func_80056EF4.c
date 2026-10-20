/* ovl201 0x80056EF4  size 208  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80056FC4(UiObj *);                        /* extern */
extern M2C_UNK D_800B6FB8;

void func_80056EF4(u8 *arg0, u8 *arg1)
{
  UiObj *temp_v0;
  int new_var;
  temp_v0 = ui_obj_alloc(6);
  temp_v0->ctor = func_80056EF4;
  temp_v0->update = func_80056FC4;
  temp_v0->destroy = ui_obj_nop;
  temp_v0->render_type = 3;
  temp_v0->light = 0xB;
  temp_v0->unk14 = 1;
  temp_v0->res = &D_800B6FB8;
  temp_v0->has_xform = 0;
  temp_v0->rot.vx = 0;
  temp_v0->rot.vy = 0;
  temp_v0->rot.vz = 0;
  temp_v0->pos.vx = ((*arg0) * 0x44) + 6;
  new_var = ((*arg1) * 0x44) + 6;
  temp_v0->pos.vz = 0;
  temp_v0->w.list.src = arg0;
  *((u8 **) (((s8 *) temp_v0) + 0x80)) = arg1;
  temp_v0->pos.vy = new_var;
}
